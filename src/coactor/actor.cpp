#include "coactor/actor.hpp"

#include "coactor/scheduler.hpp"

#include <chrono>
#include <format>
#include <iostream>

namespace coactor {

Actor::~Actor()
{
	log("Destroying");
	m_handle.destroy();
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

std::optional<detail::Envelope> Actor::resume()
{
	log("Running");
	m_status = Status::Running;
	auto& promise = m_handle.promise();
	promise.yielded_envelope = std::nullopt;
	if (!m_handle.done()) {
		m_handle.resume();
	}

	if (m_handle.done()) {
		log("Done");
		m_status = Status::Done;
	} else {
		log("Blocked");
		m_status = Status::Blocked;
	}

	return promise.yielded_envelope;
}

void Actor::append_msg(const std::string& msg)
{
	m_message_queue.push_back(msg);
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

detail::Envelope Actor::send(ActorId receiver, const std::string& msg)
{
	// log(std::format("Sending \"{}\" to {}", msg, receiver));
	// m_scheduler->send(receiver, msg);

	return detail::Envelope{receiver, msg};
}

detail::ReceiveAwaiter Actor::receive()
{
	log("Receiving");
	return detail::ReceiveAwaiter{m_message_queue};
}

Actor::Handle detail::Promise::get_return_object()
{
	return Actor::Handle::from_promise(*this);
}

std::suspend_always detail::Promise::initial_suspend()
{
	return {};
}

void detail::Promise::return_void()
{
}

void detail::Promise::unhandled_exception()
{
	try {
		std::rethrow_exception(std::current_exception());
	} catch (const std::exception& e) {
		std::cout << "Unhandled exception: " << e.what() << '\n';
	} catch (...) {
		std::cout << "Unhandled exception: Unknown error\n";
	}
}

std::suspend_always detail::Promise::final_suspend() noexcept
{
	return {};
}

} // namespace coactor
