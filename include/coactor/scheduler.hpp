#pragma once

#include "coactor/detail/actor_coro.hpp"

#include <condition_variable>
#include <list>
#include <map>
#include <memory>
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

	// Only to be run once, in its own thread. Blocks until done.
	void run();

	// Called by Runtime::run() when the scheduler is to exit.
	void exit();

private:
	void insert_incoming_actors();
	void insert_incoming_messages();

	void wait_until_incoming_or_exit();

	void log(std::string_view message);

	Runtime* m_runtime;

	std::map<Address, std::shared_ptr<Actor>> m_actors;

	std::mutex m_incoming_mutex;
	std::condition_variable m_incoming_cv;
	bool m_has_incoming{false};
	bool m_shall_exit{false};
	std::list<std::shared_ptr<Actor>> m_incoming_actors;
	std::list<std::pair<Address, std::string>> m_incoming_messages;

	std::list<Address> m_ready_queue;
};

} // namespace coactor
