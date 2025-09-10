#include "actor5.hpp"

#include "scheduler5.hpp"

namespace coactor {

Actor::~Actor()
{
	m_handle.destroy();
}

void Actor::init()
{
	m_handle = act();
}

void Actor::resume()
{
	if (!m_handle.done()) {
		m_handle.resume();
	}
}

bool Actor::is_message_queue_empty() const
{
	return m_message_queue.empty();
}

void Actor::append_msg(const std::string& msg)
{
	m_message_queue.push_back(msg);
}

void Actor::send(Handle receiver, const std::string& msg)
{
	m_scheduler->send(receiver, msg);
}

Actor::ReceiveAwaiter Actor::receive()
{
	return ReceiveAwaiter{m_message_queue};
}

} // namespace coactor
