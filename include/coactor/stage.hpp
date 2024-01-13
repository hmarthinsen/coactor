#pragma once

#include "coactor/actor.hpp"
#include "coactor/message.hpp"
#include "coactor/result.hpp"

#include <concurrencpp/concurrencpp.h>

#include <map>
#include <memory>

namespace coactor {

class Inbox;

Result<void> run_actor(
	concurrencpp::executor_tag,
	std::shared_ptr<concurrencpp::thread_pool_executor> executor,
	Stage& stage,
	ActorId actor_id,
	Actor actor,
	std::map<ActorId, std::shared_ptr<Inbox>>& inboxes,
	concurrencpp::async_lock& stage_lock,
	concurrencpp::async_condition_variable& inboxes_cv
);

class Stage {
public:
	std::shared_ptr<concurrencpp::thread_pool_executor> get_executor();

	ActorId spawn_actor(Actor actor);

	Result<void> send(ActorId recipient, const Message& message);

	void wait_until_done();

	Result<void> sleep(std::chrono::milliseconds duration);

private:
	concurrencpp::runtime m_runtime;

	ActorId m_next_id = 0;

	std::map<ActorId, std::shared_ptr<Inbox>> m_inboxes;
	concurrencpp::async_lock m_stage_lock;
	concurrencpp::async_condition_variable m_inboxes_cv;
};

} // namespace coactor
