#include "coactor/scheduler.hpp"

#include "coactor/actor.hpp"

#include <iostream>
#include <mutex>
#include <utility>

namespace coactor {

ActorId Scheduler::insert_actor(std::unique_ptr<Actor> actor)
{
	// actor->set_scheduler(this);
	actor->init();

	const auto id = actor->id();
	m_actors[id] = std::move(actor);

	m_ready_queue.push_back(id);

	return id;
}

void Scheduler::run()
{
	while (true) {
		{ // Process transfer queue.
			std::lock_guard<std::mutex> guard{m_transfer_queue_mutex};
			if (!m_transfer_queue.empty()) {
				const auto [id, msg] = m_transfer_queue.front();
				m_transfer_queue.pop_front();
				send(id, msg);
				continue;
			}
		}

		if (m_ready_queue.empty()) {
			std::cout << "Ready queue is empty!\n";
			break;
		}

		const ActorId id = m_ready_queue.front();
		m_ready_queue.pop_front();

		std::optional<detail::Envelope> e = m_actors[id]->resume();
		if (e) {
			send(e->receiver, e->msg);
		}

		if (m_actors[id]->status() == Actor::Status::Done) {
			m_actors.erase(id);
		}
	}
}

void Scheduler::send(ActorId receiver, const std::string& msg)
{
	// if (receiver.scheduler_address == this) {
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
	// } else { // Redirect to correct scheduler.
	// 	auto target_scheduler
	// 		= static_cast<Scheduler*>(receiver.scheduler_address);
	// 	target_scheduler->transfer(receiver, msg);
	// }
}

void Scheduler::transfer(ActorId receiver, const std::string& msg)
{
	std::lock_guard<std::mutex> guard{m_transfer_queue_mutex};
	m_transfer_queue.push_back({receiver, msg});
}

} // namespace coactor
