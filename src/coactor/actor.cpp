#include "coactor/actor.hpp"

#include "coactor/stage.hpp"

namespace coactor {

Actor::Actor(Stage& stage) : m_stage{stage} { }

Actor::~Actor() { m_inbox.shutdown(m_stage).get(); }

Result<int> Actor::receive()
{
	const auto executor = m_stage.get_executor();
	co_return co_await m_inbox.pop(executor);
}

Result<void> Actor::send(ActorId recipient, int i)
{
	co_await m_stage.send(recipient, i);
}

} // namespace coactor
