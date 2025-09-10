#include "scheduler4.hpp"

#include "actor4.hpp"

namespace coactor {

ActorHandle Scheduler::insert(std::unique_ptr<Actor> actor)
{
	actor->set_scheduler(this);
	actor->resume();

	const auto id = actor->id();
	m_actors[id] = std::move(actor);

	return id;
}

void Scheduler::send(ActorHandle receiver, const std::string& msg)
{
	m_actors[receiver]->append_msg(msg);
}

} // namespace coactor
