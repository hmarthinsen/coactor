#pragma once

#include "coactor/detail/actor_coro.hpp"

#include <list>
#include <map>
#include <memory>
#include <string>

namespace coactor {

class Actor;

class Scheduler {
public:
	ActorId insert_actor(std::unique_ptr<Actor> actor);

	void run();

private:
	void send(ActorId receiver, const std::string& msg);

	void log(std::string_view message);

	std::map<ActorId, std::unique_ptr<Actor>> m_actors;
	std::list<ActorId> m_ready_queue;
};

} // namespace coactor
