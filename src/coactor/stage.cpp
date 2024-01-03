#include "coactor/stage.hpp"

#include "coactor/actor.hpp"

#include <format>
#include <iostream>
#include <syncstream>

namespace coactor {

std::shared_ptr<concurrencpp::thread_pool_executor> Stage::get_executor()
{
	return m_runtime.thread_pool_executor();
}

Result<void> Stage::send(ActorId recipient, const Message& message)
{
	const auto executor = get_executor();
	auto guard = co_await m_stage_lock.lock(executor);

	if (m_actors.contains(recipient)) {
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

		co_await m_actors[recipient]->m_inbox.push(
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

	m_actors_cv.await(executor, guard, [this] { return m_actors.empty(); })
		.run()
		.get();
}

} // namespace coactor
