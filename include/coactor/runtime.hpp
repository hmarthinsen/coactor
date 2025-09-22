#pragma once

#include "coactor/detail/utils.hpp"
#include "coactor/scheduler.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <cstdint>

namespace coactor {

class Actor;
using Address = std::uint64_t;

// Thread-safe. Its methods will be called by several schedulers simultaneously.
class Runtime {
public:
	void add_schedulers(int num_schedulers);

	// Only one thread may call run().
	void run();

	template <typename ActorT, typename... Args>
	Address spawn_actor(Args... args);
	Address insert_actor(std::shared_ptr<Actor>);
	void erase_actor(Address);

	void send(Address receiver, const std::string& msg);

	// void log(Address from, std::string_view msg);

private:
	std::mutex m_actors_mutex{};
	std::map<Address, std::shared_ptr<Actor>> m_actors{};

	std::mutex m_schedulers_mutex{};
	std::vector<std::jthread> m_scheduler_threads{};
	std::vector<std::unique_ptr<Scheduler>> m_schedulers{};
	std::map<Address, Scheduler*> m_address_to_scheduler{};
	int m_next_scheduler{0}; // Index of scheduler to use on next spawn.
};

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
