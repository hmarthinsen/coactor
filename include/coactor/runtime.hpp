#pragma once

#include "coactor/detail/utils.hpp"
#include "coactor/scheduler.hpp"

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
	void send(const std::string& receiver_name, const std::string& msg);

	Actor* get_actor(Address) const;

	void register_actor_name(Address, const std::string& name);

private:
	Address insert_actor(std::shared_ptr<Actor>, std::string_view name);

	void add_schedulers();

	void wait_until_actors_done();

	unsigned int m_num_schedulers{0};

	mutable std::mutex m_actors_mutex{};
	std::map<Address, std::shared_ptr<Actor>> m_actors{};
	std::map<std::string, Address> m_name_to_addr{};
	std::condition_variable m_actors_done_cv{};

	// TODO: Each scheduler should have its own mutex, so that sending of
	// messages doesn't block the whole runtime.
	std::mutex m_schedulers_mutex{};
	std::vector<std::unique_ptr<Scheduler>> m_schedulers{};
	std::unique_ptr<Scheduler> m_stdio_scheduler{}; // For I/O only.

	int m_next_scheduler{0}; // Index of scheduler to use on next spawn.
};

template <typename ActorT, typename... Args>
int Runtime::run(Args... args)
{
	add_schedulers();
	spawn_actor<ActorT>(args...);

	std::jthread stdio_scheduler_thread{[this] { m_stdio_scheduler->run(); }};

	std::vector<std::jthread> scheduler_threads{};
	for (auto& scheduler : m_schedulers) {
		scheduler_threads.emplace_back([&scheduler] { scheduler->run(); });
	}

	wait_until_actors_done();

	for (auto& scheduler : m_schedulers) {
		scheduler->exit();
	}

	m_stdio_scheduler->exit();

	for (auto& thread : scheduler_threads) {
		thread.join();
	}

	stdio_scheduler_thread.join();

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
