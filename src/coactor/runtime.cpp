#include "coactor/runtime.hpp"

#include "coactor/actor.hpp"
#include "coactor/detail/utils.hpp"
#include "coactor/scheduler.hpp"
#include "coactor/stdio.hpp"

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

	// Add the StdIo Actor in its own Scheduler:

	auto stdio_actor = std::make_shared<StdIo>();
	const std::string stdio_name{"StdIo"};
	const Address stdio_addr = detail::get_unique_id();
	stdio_actor->init(this, stdio_addr, stdio_name);

	{
		std::lock_guard lock{m_actors_mutex};
		m_actors[stdio_addr] = stdio_actor;
		m_name_to_addr[stdio_name] = stdio_addr;
	}

	m_stdio_scheduler = std::make_unique<Scheduler>(this);
	m_stdio_scheduler->insert_actor(stdio_actor);
}

void Runtime::erase_actor(Address address)
{
	{
		std::lock_guard lock{m_actors_mutex};
		m_actors.erase(address);
		std::erase_if(m_name_to_addr, [address](const auto& item) {
			const Address addr = item.second;
			return addr == address;
		});

		m_actors_done_cv.notify_one();
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

void Runtime::send(const std::string& receiver_name, const std::string& msg)
{
	Address receiver_addr;
	{
		std::lock_guard lock{m_actors_mutex};

		if (!m_name_to_addr.contains(receiver_name)) {
			detail::log(
				"Runtime",
				detail::red(
					"Warning: Attempted to send to nonexistent actor name: "
					+ receiver_name
				)
			);
			return;
		}
		receiver_addr = m_name_to_addr.at(receiver_name);
	}

	send(receiver_addr, msg);
}

Actor* Runtime::get_actor(Address address) const
{
	std::lock_guard lock{m_actors_mutex};
	return m_actors.at(address).get();
}

void Runtime::register_actor_name(Address address, const std::string& name)
{
	std::lock_guard lock{m_actors_mutex};
	m_name_to_addr[name] = address;
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
}

void Runtime::wait_until_actors_done()
{
	std::unique_lock lock{m_actors_mutex};
	m_actors_done_cv.wait(lock, [this] {
		return m_actors.size() == 1; // Only StdIo left.
	});
}

} // namespace coactor
