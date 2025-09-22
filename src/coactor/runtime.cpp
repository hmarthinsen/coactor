#include "coactor/runtime.hpp"

#include "coactor/actor.hpp"
#include "coactor/scheduler.hpp"

#include <memory>

namespace coactor {

Address Runtime::insert_actor(std::shared_ptr<Actor> actor)
{
	Address address = actor->address();

	{
		std::lock_guard lock{m_schedulers_mutex};

		Scheduler* scheduler = m_schedulers.at(m_next_scheduler).get();
		scheduler->insert_actor(actor);
		m_address_to_scheduler[address] = scheduler;

		m_next_scheduler = (m_next_scheduler + 1) % m_schedulers.size();
	}
	{
		std::lock_guard lock{m_actors_mutex};
		m_actors[address] = std::move(actor);
	}

	return address;
}

void Runtime::erase_actor(Address address)
{
	std::lock_guard lock{m_actors_mutex};
	m_actors.erase(address);
}

void Runtime::add_schedulers(int num_schedulers)
{
	std::lock_guard lock{m_schedulers_mutex};
	for (int i = 0; i < num_schedulers; ++i) {
		m_schedulers.push_back(std::make_unique<Scheduler>(this));
	}
}

void Runtime::run()
{
	for (auto& scheduler : m_schedulers) {
		m_scheduler_threads.emplace_back([&scheduler] { scheduler->run(); });
	}

	for (auto& thread : m_scheduler_threads) {
		thread.join();
	}
}

void Runtime::send(Address receiver, const std::string& msg)
{
	{
		std::lock_guard lock{m_schedulers_mutex};
		Scheduler* scheduler = m_address_to_scheduler.at(receiver);
		scheduler->send(receiver, msg);
	}
}

// void Runtime::log(
// 	[[maybe_unused]] Address from,
// 	[[maybe_unused]] std::string_view msg
// )
// {
// #ifndef NDEBUG
// 	static const auto time_start{std::chrono::system_clock::now()};
// 	std::cout << std::format(
// 		"{} [{}] - {}\n",
// 		std::chrono::duration_cast<std::chrono::microseconds>(
// 			std::chrono::system_clock::now() - time_start
// 		),
// 		from,
// 		msg
// 	);
// #endif
// }

} // namespace coactor
