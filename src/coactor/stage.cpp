#include "coactor/stage.hpp"

#include "coactor/actor.hpp"

#include <utility>

namespace coactor {

std::shared_ptr<concurrencpp::thread_pool_executor> Stage::get_executor()
{
	return m_runtime.thread_pool_executor();
}

ActorId Stage::spawn_actor(std::function<Result<void>(Stage&, Actor&)> fun)
{
	auto actor_id = m_next_id++;
	auto actor = std::make_unique<Actor>(*this, fun);
	m_actors.emplace(actor_id, std::move(actor));

	return actor_id;
}

} // namespace coactor