#include "actor4.hpp"

#include "scheduler4.hpp"

#include <format>

namespace coactor {

Actor::~Actor()
{
	m_handle.destroy();
}

void Actor::resume()
{
	if (!m_handle) {
		m_handle = act();
	}

	if (!m_handle.done()) {
		m_handle.resume();
	}
}

Actor::Handle Actor::id() const
{
	std::cout << std::format("Actor address: {}\n", m_handle.address());
	return m_handle;
}

void Actor::set_scheduler(Scheduler* scheduler)
{
	m_scheduler = scheduler;
}

void Actor::append_msg(const std::string& msg)
{
	m_message_queue.push_back(msg);
}

void Actor::send(Handle receiver, const std::string& msg)
{
	std::cout << std::format("Sending message: {}\n", msg);
	m_scheduler->send(receiver, msg);
}

Actor::ReceiveAwaiter Actor::receive()
{
	return {m_message_queue};
}

} // namespace coactor
