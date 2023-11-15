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
	co_await m_actors[recipient]->m_inbox.push(executor, i);
}

Result<void> Stage::run()
{
	std::vector<Result<void>> results;
	for (const auto& [actor_id, actor] : m_actors) {
		results.push_back(actor->run());
	}

	co_await concurrencpp::when_all(get_executor(), results.begin(), results.end());
}

} // namespace coactor