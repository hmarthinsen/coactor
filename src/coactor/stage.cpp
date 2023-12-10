#include "coactor/stage.hpp"

#include "coactor/actor.hpp"

namespace coactor {

std::shared_ptr<concurrencpp::thread_pool_executor> Stage::get_executor()
{
	return m_runtime.thread_pool_executor();
}

Result<void> Stage::send(ActorId recipient, int i)
{
	const auto executor = get_executor();
	auto guard = co_await m_stage_lock.lock(executor);

	if (m_actors.contains(recipient)) {
		const auto executor = get_executor();
		co_await m_actors[recipient]->m_inbox.push(executor, i);
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
