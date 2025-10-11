#include "coactor/scheduler.hpp"

#include "coactor/actor.hpp"
#include "coactor/runtime.hpp"

#include <chrono>
#include <mutex>

namespace coactor {

void Scheduler::insert_actor(std::shared_ptr<Actor> actor)
{
	const Address address = actor->address();

	auto set_ready = [this, address] {
		if (m_timeouts_reverse.contains(address)) {
			// Remove registered timeout.
			auto timeout = m_timeouts_reverse[address];
			m_timeouts.erase(timeout);
			m_timeouts_reverse.erase(address);
		}

		{
			std::lock_guard lock{m_ready_queue_mutex};
			m_ready_queue.push_back(address);
		}

		std::unique_lock lock{m_ready_mutex};
		m_is_ready = true;
		m_ready_cv.notify_one();
	};

	actor->register_ready_callback(set_ready);

	set_ready();
}

void Scheduler::run()
{
	while (true) {
		// Wait until an actor is ready or exit:
		if (is_ready_queue_empty()) {
			{
				std::unique_lock lock{m_ready_mutex};
				m_is_ready = false;
			}

			if (m_timeouts.empty()) {
				wait_until_ready_or_exit();
			} else {
				wait_until_ready_or_exit(m_timeouts.begin()->first);
			}

			std::unique_lock lock{m_ready_mutex};
			if (m_shall_exit) {
				return;
			}
		}

		Address address = pop_ready_queue();
		Actor* actor = m_runtime->get_actor(address);
		actor->resume();

		if (actor->status() == Actor::Status::Done) {
			m_runtime->erase_actor(address);
			continue;
		}

		// Register timeout if there is one:
		if (auto timeout_point = actor->timeout_point()) {
			// FIXME: Hack: Nudge timeout_point until it is unique.
			while (m_timeouts.contains(*timeout_point)) {
				(*timeout_point) += std::chrono::steady_clock::duration::min();
			}
			m_timeouts[*timeout_point] = {address, actor->timeout_msg()};
			m_timeouts_reverse[address] = *timeout_point;
		}

		// Handle timeouts:
		const auto now = std::chrono::steady_clock::now();
		for (auto item = m_timeouts.begin(); item != m_timeouts.end();) {
			const auto& [timeout_point, envelope] = *item;
			if (now < timeout_point) {
				// No timeout
				break;
			}

			const auto& [address, timeout_msg] = envelope;
			m_runtime->send(address, timeout_msg);

			item = m_timeouts.erase(item);
			m_timeouts_reverse.erase(address);
		}
	}
}

void Scheduler::exit()
{
	{
		std::unique_lock lock{m_ready_mutex};
		m_shall_exit = true;
	}
	m_ready_cv.notify_one();
}

bool Scheduler::is_ready_queue_empty() const
{
	std::lock_guard lock{m_ready_queue_mutex};
	return m_ready_queue.empty();
}

Address Scheduler::pop_ready_queue()
{
	std::lock_guard lock{m_ready_queue_mutex};
	Address address = m_ready_queue.front();
	m_ready_queue.pop_front();

	return address;
}

void Scheduler::wait_until_ready_or_exit(
	std::optional<std::chrono::time_point<std::chrono::steady_clock>>
		timeout_point
)
{
	const auto pred = [this] { return m_is_ready || m_shall_exit; };

	if (timeout_point) {
		bool is_timeout;
		{
			std::unique_lock lock{m_ready_mutex};
			is_timeout = !m_ready_cv.wait_until(lock, *timeout_point, pred);
		}
		if (is_timeout) {
			const auto& [address, timeout_msg] = m_timeouts[*timeout_point];

			m_runtime->send(address, timeout_msg);

			m_timeouts.erase(*timeout_point);
			m_timeouts_reverse.erase(address);
		}
	} else {
		std::unique_lock lock{m_ready_mutex};
		m_ready_cv.wait(lock, pred);
	}
}

} // namespace coactor
