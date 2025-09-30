#include "coactor/scheduler.hpp"

#include "coactor/actor.hpp"
#include "coactor/runtime.hpp"

#include <mutex>
#include <utility>

namespace coactor {

void Scheduler::insert_actor(std::shared_ptr<Actor> actor)
{
	std::unique_lock lock{m_incoming_mutex};
	m_incoming_actors.push_back(std::move(actor));
	m_has_incoming = true;

	m_incoming_cv.notify_one();
}

void Scheduler::insert_message(Address receiver, const std::string& msg)
{
	std::unique_lock lock{m_incoming_mutex};
	m_incoming_messages.push_back({receiver, msg});
	m_has_incoming = true;

	m_incoming_cv.notify_one();
}

void Scheduler::run()
{
	while (true) {
		insert_incoming_actors();
		insert_incoming_messages();

		if (m_ready_queue.empty()) {
			m_runtime->signal_scheduler_blocked();

			wait_until_incoming_or_exit();
			if (m_shall_exit) {
				return;
			}

			m_runtime->signal_scheduler_unblocked();
			continue;
		}

		const Address active_actor_addr = m_ready_queue.front();
		m_ready_queue.pop_front();
		Actor* active_actor = m_actors[active_actor_addr].get();

		active_actor->resume();

		if (active_actor->status() == Actor::Status::Done) {
			m_runtime->erase_actor(active_actor_addr);
			m_actors.erase(active_actor_addr);
		}
	}
}

void Scheduler::exit()
{
	{
		std::unique_lock lock{m_incoming_mutex};
		m_shall_exit = true;
	}
	m_incoming_cv.notify_one();
}

void Scheduler::insert_incoming_actors()
{
	std::lock_guard lock{m_incoming_mutex};

	m_has_incoming = false;

	for (const auto& actor : m_incoming_actors) {
		const auto address = actor->address();
		m_actors[address] = std::move(actor);

		m_ready_queue.push_back(address);
	}

	m_incoming_actors.clear();
}

void Scheduler::insert_incoming_messages()
{
	std::lock_guard lock{m_incoming_mutex};

	m_has_incoming = false;

	for (const auto& [receiver, msg] : m_incoming_messages) {
		switch (m_actors[receiver]->status()) {
		case Actor::Status::Ready:
		case Actor::Status::Running:
			m_actors[receiver]->append_msg(msg);
			break;
		case Actor::Status::Blocked:
			m_actors[receiver]->append_msg(msg);
			m_actors[receiver]->set_ready();
			m_ready_queue.push_back(receiver);
			break;
		case Actor::Status::Done:
			break;
		}
	}

	m_incoming_messages.clear();
}

void Scheduler::wait_until_incoming_or_exit()
{
	std::unique_lock lock{m_incoming_mutex};
	m_incoming_cv.wait(lock, [this] { return m_has_incoming || m_shall_exit; });
}

} // namespace coactor
