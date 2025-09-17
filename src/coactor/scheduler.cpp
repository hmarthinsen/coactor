#include "coactor/scheduler.hpp"

#include "coactor/actor.hpp"
#include "coactor/detail/actor_coro.hpp"
#include "coactor/detail/utils.hpp"

#include <utility>
#include <variant>

namespace coactor {

Address Scheduler::insert_actor(std::unique_ptr<Actor> actor)
{
	actor->init();

	const auto address = actor->address();
	m_actors[address] = std::move(actor);

	m_ready_queue.push_back(address);

	return address;
}

void Scheduler::run()
{
	while (true) {
		if (m_ready_queue.empty()) {
			log(detail::bold("Ready queue empty"));
			break;
		}

		const Address active_actor_addr = m_ready_queue.front();
		m_ready_queue.pop_front();
		Actor* active_actor = m_actors[active_actor_addr].get();

		detail::SchedulerCommand cmd = active_actor->resume();

		if (std::holds_alternative<detail::SendCommand>(cmd)) {
			auto send_cmd = std::get<detail::SendCommand>(cmd);
			send(send_cmd.receiver, send_cmd.msg);

			active_actor->set_ready();
			m_ready_queue.push_back(active_actor_addr);
		} else if (std::holds_alternative<detail::SpawnCommand>(cmd)) {
			auto spawn_cmd = std::move(std::get<detail::SpawnCommand>(cmd));
			insert_actor(std::move(spawn_cmd.actor));

			active_actor->set_ready();
			m_ready_queue.push_back(active_actor_addr);
		}

		if (active_actor->status() == Actor::Status::Done) {
			m_actors.erase(active_actor_addr);
		}
	}
}

void Scheduler::send(Address receiver, const std::string& msg)
{
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
	detail::log("Scheduler", message);
}

} // namespace coactor
