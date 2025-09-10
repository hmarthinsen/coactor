#include "actor6.hpp"

#include "scheduler6.hpp"

#include <chrono>

namespace coactor {

Actor::~Actor()
{
	std::cout << "Destroying " << m_name << '\n';
	m_handle.destroy();
}

void Actor::log(const std::string& message)
{
	std::cout << std::format(
		"{} [{}] {}\n",
		std::chrono::system_clock::now(),
		m_name,
		message
	);
}

void Actor::init()
{
	log("Initializing");
	m_handle = act();
}

void Actor::set_ready()
{
	log("Ready");
	m_status = Status::Ready;
}

void Actor::resume()
{
	log("Running");
	m_status = Status::Running;
	m_handle.resume();

	if (m_handle.done()) {
		log("Done");
		m_status = Status::Done;
	} else {
		log("Blocked");
		m_status = Status::Blocked;
	}
}

void Actor::append_msg(const std::string& msg)
{
	m_message_queue.push_back(msg);
}

void Actor::send(ActorId receiver, const std::string& msg)
{
	log(std::format("Sending \"{}\" to {}", msg, receiver));
	m_scheduler->send(receiver, msg);
}

Actor::ReceiveAwaiter Actor::receive()
{
	log("Receiving");
	return ReceiveAwaiter{m_message_queue};
}

} // namespace coactor
