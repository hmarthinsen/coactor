#include "coactor/scheduler.hpp"

#include "coactor/actor.hpp"
#include "coactor/detail/utils.hpp"

#include <chrono>
#include <format>
#include <iostream>
#include <utility>
#include <variant>

namespace coactor {

ActorId Scheduler::insert_actor(std::unique_ptr<Actor> actor)
{
	actor->init();

	const auto id = actor->id();
	m_actors[id] = std::move(actor);

	m_ready_queue.push_back(id);

	return id;
}

void Scheduler::run()
{
	while (true) {
		if (m_ready_queue.empty()) {
			// log("Ready queue is empty!");
			break;
		}

		const ActorId id = m_ready_queue.front();
		m_ready_queue.pop_front();

		detail::SchedulerCommand cmd = m_actors[id]->resume();

		if (std::holds_alternative<detail::SendCommand>(cmd)) {
			auto send_cmd = std::get<detail::SendCommand>(cmd);
			send(send_cmd.receiver, send_cmd.msg);
			m_ready_queue.push_back(id);
		} else if (std::holds_alternative<detail::SpawnCommand>(cmd)) {
			auto spawn_cmd = std::move(std::get<detail::SpawnCommand>(cmd));
			insert_actor(std::move(spawn_cmd.actor));
			m_ready_queue.push_back(id);
		}

		if (m_actors[id]->status() == Actor::Status::Done) {
			m_actors.erase(id);
		}
	}
}

void Scheduler::send(ActorId receiver, const std::string& msg)
{
	// log(std::format("Sending to {}: {}", m_actors[receiver]->name(), msg));

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

void Scheduler::log(std::string_view message)
{
	std::cout << std::format(
		"{} [Scheduler] {}\n",
		std::chrono::system_clock::now(),
		message
	);
}

} // namespace coactor
