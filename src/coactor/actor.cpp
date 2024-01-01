#include "coactor/actor.hpp"

#include "coactor/stage.hpp"

namespace coactor {

Actor::Actor(Stage& stage, ActorId id) : m_stage{stage}, m_id{id} { }

Actor::~Actor() { m_inbox.shutdown(m_stage).get(); }

Result<Message> Actor::receive()
{
	const auto executor = m_stage.get_executor();
	co_return Message::from_msgpack(co_await m_inbox.pop(executor));
}

Result<void> Actor::send(ActorId recipient, const Message& message)
{
	co_await m_stage.send(recipient, message);
}

ActorId Actor::get_actor_id() { return m_id; }

} // namespace coactor
