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
		// Remove any registered timeout.
		if (m_timeouts_reverse.contains(address)) {
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

		// Wait until an actor is ready:
		Address active_actor_addr;
		{
			bool is_ready_queue_empty;
			{
				std::lock_guard lock{m_ready_queue_mutex};
				is_ready_queue_empty = m_ready_queue.empty();
			}
			if (is_ready_queue_empty) {
				{
					std::unique_lock lock{m_ready_mutex};
					m_is_ready = false;
				}

				if (m_timeouts.empty()) {
					m_runtime->notify_scheduler_blocked();
					wait_until_ready_or_exit();
					m_runtime->notify_scheduler_unblocked();
				} else {
					wait_until_ready_or_exit(m_timeouts.begin()->first);
				}

				std::unique_lock lock{m_ready_mutex};
				if (m_shall_exit) {
					return;
				}
			}

			std::lock_guard lock{m_ready_queue_mutex};
			active_actor_addr = m_ready_queue.front();
			m_ready_queue.pop_front();
		}

		Actor* active_actor = m_runtime->get_actor(active_actor_addr);
		active_actor->resume();

		if (active_actor->status() == Actor::Status::Done) {
			m_runtime->erase_actor(active_actor_addr);
			continue;
		}

		auto timeout_point = active_actor->timeout_point();
		const auto timeout_msg = active_actor->timeout_msg();
		if (timeout_point) {
			// FIXME: Hack: Nudge timeout_point until it is unique.
			while (m_timeouts.contains(*timeout_point)) {
				(*timeout_point) += std::chrono::steady_clock::duration::min();
			}
			m_timeouts[*timeout_point] = {active_actor_addr, timeout_msg};
			m_timeouts_reverse[active_actor_addr] = *timeout_point;
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
