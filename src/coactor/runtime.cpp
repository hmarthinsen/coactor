#include "coactor/runtime.hpp"

#include "coactor/actor.hpp"
#include "coactor/scheduler.hpp"

#include <memory>
#include <mutex>

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

	m_num_schedulers_unblocked = num_schedulers;
}

void Runtime::run()
{
	for (auto& scheduler : m_schedulers) {
		m_scheduler_threads.emplace_back([&scheduler] { scheduler->run(); });
	}

	wait_until_schedulers_blocked();

	for (auto& scheduler : m_schedulers) {
		scheduler->exit();
	}

	detail::log("Runtime", "Done");
}

void Runtime::send(Address receiver, const std::string& msg)
{
	std::lock_guard lock{m_schedulers_mutex};
	Scheduler* scheduler = m_address_to_scheduler.at(receiver);
	scheduler->send(receiver, msg);
}

void Runtime::signal_scheduler_blocked()
{
	std::unique_lock lock{m_num_schedulers_unblocked_mutex};
	m_num_schedulers_unblocked--;

	m_num_schedulers_unblocked_cv.notify_one();
}

void Runtime::signal_scheduler_unblocked()
{
	std::unique_lock lock{m_num_schedulers_unblocked_mutex};
	m_num_schedulers_unblocked++;
}

void Runtime::wait_until_schedulers_blocked()
{
	std::unique_lock lock{m_num_schedulers_unblocked_mutex};
	m_num_schedulers_unblocked_cv.wait(lock, [this] {
		return m_num_schedulers_unblocked == 0;
	});
}

} // namespace coactor
