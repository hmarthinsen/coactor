#pragma once

#include <condition_variable>
#include <list>
#include <map>
#include <memory>
#include <optional>

namespace coactor {

class Actor;
class Runtime;

using Address = std::uint64_t;

// Most of the methods are designed to be used only by a single thread.
class Scheduler {
public:
	Scheduler(Runtime* runtime) : m_runtime{runtime} { }

	// Thread-safe. Used by the runtime.
	void insert_actor(std::shared_ptr<Actor> actor);

	// Only to be run once, in its own thread. Blocks until done.
	void run();

	// Called by Runtime::run() when the scheduler is to exit.
	void exit();

private:
	bool is_ready_queue_empty() const;
	Address pop_ready_queue();

	void wait_until_ready_or_exit(
		std::optional<std::chrono::time_point<std::chrono::steady_clock>>
			timeout_point
		= std::nullopt
	);

	Runtime* m_runtime;

	std::mutex m_ready_mutex;
	std::condition_variable m_ready_cv;
	bool m_is_ready{false};
	bool m_shall_exit{false};
	std::map<
		std::chrono::time_point<std::chrono::steady_clock>,
		std::pair<Address, std::string>>
		m_timeouts;
	std::map<Address, std::chrono::time_point<std::chrono::steady_clock>>
		m_timeouts_reverse;

	mutable std::mutex m_ready_queue_mutex;
	std::list<Address> m_ready_queue;
};

} // namespace coactor
