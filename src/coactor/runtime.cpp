#include "coactor/runtime.hpp"

#include "coactor/actor.hpp"
#include "coactor/detail/utils.hpp"
#include "coactor/scheduler.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

namespace coactor {

Runtime::Runtime(unsigned int num_schedulers) : m_num_schedulers{num_schedulers}
{
	if (num_schedulers == 0) {
		m_num_schedulers = std::thread::hardware_concurrency();
	}
}

Address
Runtime::insert_actor(std::shared_ptr<Actor> actor, std::string_view name)
{
	const Address address = detail::get_unique_id();
	actor->init(this, address, name);

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
	{
		std::lock_guard lock{m_actors_mutex};
		m_actors.erase(address);
	}
	{
		std::lock_guard lock{m_schedulers_mutex};
		m_address_to_scheduler.erase(address);
	}
}

void Runtime::add_schedulers()
{
	std::lock_guard lock{m_schedulers_mutex};
	for (unsigned int i = 0; i < m_num_schedulers; ++i) {
		m_schedulers.push_back(std::make_unique<Scheduler>(this));
	}

	m_num_schedulers_unblocked = m_num_schedulers;
}

void Runtime::send(Address receiver, const std::string& msg)
{
	std::lock_guard lock{m_schedulers_mutex};
	if (!m_address_to_scheduler.contains(receiver)) {
		detail::log(
			"Runtime",
			detail::red(
				"Warning: Attempted to send to nonexistent actor: "
				+ std::to_string(receiver)
			)
		);
		return;
	}
	Scheduler* scheduler = m_address_to_scheduler.at(receiver);
	scheduler->insert_message(receiver, msg);
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
