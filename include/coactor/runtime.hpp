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

#include <cstdint>
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
	Address insert_actor(std::shared_ptr<Actor>);
	void erase_actor(Address);

	void send(Address receiver, const std::string& msg);

	void signal_scheduler_blocked();
	void signal_scheduler_unblocked();

private:
	void add_schedulers();

	// To be called in the main thread.
	void wait_until_schedulers_blocked();

	unsigned int m_num_schedulers{0};

	std::mutex m_actors_mutex{};
	std::map<Address, std::shared_ptr<Actor>> m_actors{};

	std::mutex m_schedulers_mutex{};
	std::vector<std::jthread> m_scheduler_threads{};
	std::vector<std::unique_ptr<Scheduler>> m_schedulers{};
	std::map<Address, Scheduler*> m_address_to_scheduler{};
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

	for (auto& scheduler : m_schedulers) {
		m_scheduler_threads.emplace_back([&scheduler] { scheduler->run(); });
	}

	wait_until_schedulers_blocked();

	for (auto& scheduler : m_schedulers) {
		scheduler->exit();
	}

	for (auto& thread : m_scheduler_threads) {
		thread.join();
	}

	detail::log("Runtime", "Done");
	return EXIT_SUCCESS;
}

template <typename ActorT, typename... Args>
Address Runtime::spawn_actor(Args... args)
{
	const std::string_view actor_type_name = detail::get_type_name<ActorT>();
	auto actor = std::make_shared<ActorT>(args...);
	actor->set_name(actor_type_name);

	Address address = detail::get_unique_id();
	actor->set_address(address);

	return insert_actor(std::move(actor));
}

} // namespace coactor
