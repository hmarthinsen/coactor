#pragma once

#include "actor.hpp"

#include <concurrencpp/concurrencpp.h>

#include <map>
#include <memory>
#include <utility>

namespace coactor {

class Actor;

class Stage {
public:
	std::shared_ptr<concurrencpp::thread_pool_executor> get_executor();

	template <typename ActorT, typename... Params>
	ActorId spawn_actor(Params&&... params)
	{
		const auto actor_id = m_next_id++;

		m_actors[actor_id]
			= std::make_unique<ActorT>(*this, std::forward<Params>(params)...);

		const auto executor = get_executor();

		return actor_id;
	}

	Result<void> send(ActorId recipient, int i);

	Result<void> run();

private:
	concurrencpp::runtime m_runtime;

	ActorId m_next_id = 0;
	std::map<ActorId, std::unique_ptr<Actor>> m_actors;
};

} // namespace coactor
