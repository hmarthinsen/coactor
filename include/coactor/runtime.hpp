#pragma once

#include "coactor/detail/utils.hpp"
#include "coactor/scheduler.hpp"

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <cstdlib>

namespace coactor {

class Actor;
using Address = std::uint64_t;

// Thread-safe. Its methods will be called by several schedulers simultaneously.
class Runtime {
public:
	Runtime(unsigned int num_schedulers = 0);

	// Only the main thread may call run(). Blocks until done.
	template <typename ActorT, typename... Args>
	int run(Args... args);

	template <typename ActorT, typename... Args>
	Address spawn_actor(Args... args);
	void erase_actor(Address);

	void send(Address receiver, const std::string& msg);

	// When a scheduler is blocked, it calls this method.
	void notify_scheduler_blocked();
	// When a scheduler is unblocked, it calls this method.
	void notify_scheduler_unblocked();

	Actor* get_actor(Address) const;

private:
	Address insert_actor(std::shared_ptr<Actor>, std::string_view name);

	void add_schedulers();

	void wait_until_schedulers_blocked();

	unsigned int m_num_schedulers{0};

	mutable std::mutex m_actors_mutex{};
	std::map<Address, std::shared_ptr<Actor>> m_actors{};

	// TODO: Each scheduler should have its own mutex, so that sending of
	// messages doesn't block the whole runtime.
	std::mutex m_schedulers_mutex{};
	std::vector<std::unique_ptr<Scheduler>> m_schedulers{};

	int m_next_scheduler{0}; // Index of scheduler to use on next spawn.

	std::mutex m_num_schedulers_unblocked_mutex{};
	std::condition_variable m_num_schedulers_unblocked_cv{};
	int m_num_schedulers_unblocked{};
};

template <typename ActorT, typename... Args>
int Runtime::run(Args... args)
{
	add_schedulers();
	spawn_actor<ActorT>(args...);

	std::vector<std::jthread> scheduler_threads{};
	for (auto& scheduler : m_schedulers) {
		scheduler_threads.emplace_back([&scheduler] { scheduler->run(); });
	}

	wait_until_schedulers_blocked();

	for (auto& scheduler : m_schedulers) {
		scheduler->exit();
	}

	for (auto& thread : scheduler_threads) {
		thread.join();
	}

	return EXIT_SUCCESS;
}

template <typename ActorT, typename... Args>
Address Runtime::spawn_actor(Args... args)
{
	auto actor = std::make_shared<ActorT>(args...);
	const std::string_view name = detail::get_type_name<ActorT>();

	return insert_actor(std::move(actor), name);
}

} // namespace coactor
