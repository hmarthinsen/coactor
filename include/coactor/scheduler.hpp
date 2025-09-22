#pragma once

#include "coactor/detail/actor_coro.hpp"

#include <list>
#include <map>
#include <memory>
#include <semaphore>
#include <string>

namespace coactor {

class Actor;
class Runtime;

// Most of the methods are designed to be used only by a single thread.
class Scheduler {
public:
	Scheduler(Runtime* runtime) : m_runtime{runtime} { }

	// Thread-safe.
	void insert_actor(std::shared_ptr<Actor> actor);
	// Thread-safe.
	void send(Address receiver, const std::string& msg);

	// Only to be run once, in its own thread.
	void run();

private:
	void insert_incoming_actors();
	void insert_incoming_messages();

	void log(std::string_view message);

	Runtime* m_runtime;

	std::map<Address, std::shared_ptr<Actor>> m_actors;

	std::binary_semaphore m_incoming_semaphore{0};

	std::mutex m_incoming_actors_mutex{};
	std::list<std::shared_ptr<Actor>> m_incoming_actors;

	std::mutex m_incoming_messages_mutex{};
	std::list<std::pair<Address, std::string>> m_incoming_messages;

	std::list<Address> m_ready_queue;
};

} // namespace coactor
