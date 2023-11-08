#include "stage.hpp"

#include "actor.hpp"

namespace coactor {

std::shared_ptr<concurrencpp::thread_pool_executor> Stage::get_executor()
{
	return m_runtime.thread_pool_executor();
}

Actor Stage::spawn_actor(std::function<Result<void>(Stage&, Actor&)> fun)
{
	return Actor{*this, fun};
}

} // namespace coactor