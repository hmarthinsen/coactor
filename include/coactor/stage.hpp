#pragma once

#include "coactor/actor.hpp"
#include "coactor/message.hpp"

#include <concurrencpp/concurrencpp.h>

#include <map>
#include <memory>

namespace coactor {

template <typename ActorT>
Result<void> run_actor(
	concurrencpp::executor_tag,
	std::shared_ptr<concurrencpp::thread_pool_executor> executor,
	ActorId actor_id,
	std::shared_ptr<ActorT> actor,
	std::map<ActorId, std::shared_ptr<Actor>>& actors,
	concurrencpp::async_lock& stage_lock,
	concurrencpp::async_condition_variable& actors_cv
)
{
	co_await actor->run();

	{
		auto guard = co_await stage_lock.lock(executor);
		actors.erase(actor_id);
	}

	actors_cv.notify_one();
}

class Actor;

class Stage {
public:
	std::shared_ptr<concurrencpp::thread_pool_executor> get_executor();

	template <typename ActorT, typename... Params>
	ActorId spawn_actor(Params&&... params)
	{
		const auto executor = get_executor();

		ActorId actor_id;
		std::shared_ptr<ActorT> actor;
		{
			auto guard = m_stage_lock.lock(executor).run().get();

			actor_id = m_next_id++;
			actor = std::make_shared<ActorT>(*this, actor_id, params...);
			m_actors[actor_id] = actor;
		}

		run_actor(
			{}, executor, actor_id, actor, m_actors, m_stage_lock, m_actors_cv
		);

		return actor_id;
	}

	Result<void> send(ActorId recipient, const Message& message);

	void wait_until_done();

private:
	concurrencpp::runtime m_runtime;

	ActorId m_next_id = 0;

	std::map<ActorId, std::shared_ptr<Actor>> m_actors;
	concurrencpp::async_lock m_stage_lock;
	concurrencpp::async_condition_variable m_actors_cv;
};

} // namespace coactor
