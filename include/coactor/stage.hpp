#pragma once

#include <concurrencpp/concurrencpp.h>

#include <functional>
#include <map>
#include <memory>

namespace coactor {

template <typename T> using Result = concurrencpp::result<T>;

using ActorId = int;

class Actor;

class Stage {
public:
	std::shared_ptr<concurrencpp::thread_pool_executor> get_executor();

	ActorId spawn_actor(std::function<Result<void>(Stage&, Actor&)> fun);

private:
	concurrencpp::runtime m_runtime;

	ActorId m_next_id = 0;
	std::map<ActorId, std::unique_ptr<Actor>> m_actors;
};

} // namespace coactor
