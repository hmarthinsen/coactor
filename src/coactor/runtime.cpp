#include "coactor/runtime.hpp"

#include "coactor/actor.hpp"
#include "coactor/detail/utils.hpp"
#include "coactor/scheduler.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>

namespace coactor {

Runtime::Runtime(unsigned int num_schedulers) : m_num_schedulers{num_schedulers}
{
	if (num_schedulers == 0) {
		m_num_schedulers = std::thread::hardware_concurrency();
	}
}

void Runtime::erase_actor(Address address)
{
	{
		std::lock_guard lock{m_actors_mutex};
		m_actors.erase(address);
	}
}

void Runtime::send(Address receiver_addr, const std::string& msg)
{
	std::shared_ptr<Actor> receiver;
	{
		std::lock_guard lock{m_actors_mutex};
		if (!m_actors.contains(receiver_addr)) {
			detail::log(
				"Runtime",
				detail::red(
					"Warning: Attempted to send to nonexistent actor: "
					+ std::to_string(receiver_addr)
				)
			);
			return;
		}
		receiver = m_actors.at(receiver_addr);
	}
	receiver->append_msg(msg);
}

void Runtime::notify_scheduler_blocked()
{
	std::unique_lock lock{m_num_schedulers_unblocked_mutex};
	m_num_schedulers_unblocked--;

	m_num_schedulers_unblocked_cv.notify_one();
}

void Runtime::notify_scheduler_unblocked()
{
	std::unique_lock lock{m_num_schedulers_unblocked_mutex};
	m_num_schedulers_unblocked++;
}

Actor* Runtime::get_actor(Address address) const
{
	std::lock_guard lock{m_actors_mutex};
	return m_actors.at(address).get();
}

Address
Runtime::insert_actor(std::shared_ptr<Actor> actor, std::string_view name)
{
	const Address address = detail::get_unique_id();
	actor->init(this, address, name);
	{
		std::lock_guard lock{m_actors_mutex};
		m_actors[address] = actor;
	}

	Scheduler* scheduler;
	{
		std::lock_guard lock{m_schedulers_mutex};
		scheduler = m_schedulers.at(m_next_scheduler).get();
		m_next_scheduler = (m_next_scheduler + 1) % m_schedulers.size();
	}

	scheduler->insert_actor(actor);

	return address;
}

void Runtime::add_schedulers()
{
	std::lock_guard lock{m_schedulers_mutex};
	for (unsigned int i = 0; i < m_num_schedulers; ++i) {
		m_schedulers.push_back(std::make_unique<Scheduler>(this));
	}

	m_num_schedulers_unblocked = m_num_schedulers;
}

void Runtime::wait_until_schedulers_blocked()
{
	std::unique_lock lock{m_num_schedulers_unblocked_mutex};
	m_num_schedulers_unblocked_cv.wait(lock, [this] {
		return m_num_schedulers_unblocked == 0;
	});
}

} // namespace coactor
