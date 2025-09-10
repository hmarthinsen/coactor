#pragma once

#include <coroutine>
#include <map>
#include <memory>
#include <string>

namespace coactor {

class Promise;
class Actor;

using ActorHandle = std::coroutine_handle<Promise>;

class Scheduler {
public:
	ActorHandle insert(std::unique_ptr<Actor> actor);

	void send(ActorHandle receiver, const std::string& msg);

private:
	std::map<ActorHandle, std::unique_ptr<Actor>> m_actors;
};

} // namespace coactor
