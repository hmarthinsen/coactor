#include "scheduler5.hpp"

#include "actor5.hpp"

#include <utility>

namespace coactor {

ActorHandle Scheduler::insert(std::unique_ptr<Actor> actor)
{
	actor->set_scheduler(this);
	actor->init();

	const auto id = actor->id();
	m_actors[id] = std::move(actor);

	return id;
}

void Scheduler::run()
{
	// Start all actors, then resume those with message.
	for (auto& [_, actor] : m_actors) {
		actor->resume();
	}

	while (true) {
		if (m_actors.empty()) {
			break;
		}

		std::erase_if(m_actors, [](const auto& item) {
			const auto& [id, actor] = item;
			return actor->is_done();
		});

		for (auto& [id, actor] : m_actors) {
			if (actor->is_message_queue_empty()) {
				continue;
			}
			actor->resume();
		}
	}
}

void Scheduler::send(ActorHandle receiver, const std::string& msg)
{
	m_actors[receiver]->append_msg(msg);
}

} // namespace coactor
