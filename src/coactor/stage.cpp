#include "coactor/stage.hpp"

#include "coactor/inbox.hpp"
#include "coactor/message.hpp"

#include <format>
#include <iostream>
#include <syncstream>

namespace coactor {

Result<void> run_actor(
	concurrencpp::executor_tag,
	std::shared_ptr<concurrencpp::thread_pool_executor> executor,
	Stage& stage,
	ActorId actor_id,
	Actor actor,
	std::map<ActorId, std::shared_ptr<Inbox>>& inboxes,
	concurrencpp::async_lock& stage_lock,
	concurrencpp::async_condition_variable& inboxes_cv
)
{
	co_await actor(stage, actor_id, *inboxes.at(actor_id));
	{
		auto guard = co_await stage_lock.lock(executor);
		inboxes.erase(actor_id);
	}

	inboxes_cv.notify_one();
}

std::shared_ptr<concurrencpp::thread_pool_executor> Stage::get_executor()
{
	return m_runtime.thread_pool_executor();
}

ActorId Stage::spawn_actor(Actor actor)
{
	const auto executor = get_executor();

	ActorId actor_id;
	{
		auto guard = m_stage_lock.lock(executor).run().get();

		actor_id = m_next_id++;
		auto inbox = std::make_shared<Inbox>(*this);
		m_inboxes.emplace(actor_id, inbox);
	}

	run_actor(
		{},
		executor,
		*this,
		actor_id,
		actor,
		m_inboxes,
		m_stage_lock,
		m_inboxes_cv
	);

	return actor_id;
}

Result<void> Stage::send(ActorId recipient, const Message& message)
{
	const auto executor = get_executor();
	auto guard = co_await m_stage_lock.lock(executor);

	if (m_inboxes.contains(recipient)) {
		const auto executor = get_executor();

		{
			std::osyncstream sync_out(std::cout);
			sync_out << "stage: sending to recipient " << recipient
					 << " data: ";

			for (int d : Message::to_msgpack(message)) {
				sync_out << std::hex << std::setfill('0') << std::setw(2) << d
						 << " ";
			}

			sync_out << std::endl;
		}

		co_await m_inboxes[recipient]->m_queue.push(
			executor, Message::to_msgpack(message)
		);
		std::osyncstream(std::cout) << "stage: sending to recipient "
									<< recipient << " done" << std::endl;
	}
}

void Stage::wait_until_done()
{
	const auto executor = get_executor();
	auto guard = m_stage_lock.lock(executor).run().get();

	m_inboxes_cv.await(executor, guard, [this] { return m_inboxes.empty(); })
		.run()
		.get();
}

Result<void> Stage::sleep(std::chrono::milliseconds duration)
{
	auto timer_queue = m_runtime.timer_queue();
	const auto executor = get_executor();

	co_await timer_queue->make_delay_object(duration, executor);
}

} // namespace coactor
