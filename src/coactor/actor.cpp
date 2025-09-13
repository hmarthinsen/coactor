#include "coactor/actor.hpp"

#include "coactor/detail/utils.hpp"
#include "coactor/scheduler.hpp"

#include <chrono>
#include <format>
#include <iostream>
#include <variant>

namespace coactor {

Actor::~Actor()
{
	// log("Destroying");
	m_handle.destroy();
}

void Actor::init()
{
	m_handle = act();
	// log("Initializing");
}

void Actor::set_ready()
{
	// log("Ready");
	m_status = Status::Ready;
}

detail::SchedulerCommand Actor::resume()
{
	// log("Running");
	m_status = Status::Running;

	auto& promise = m_handle.promise();
	promise.reset();

	if (!m_handle.done()) {
		m_handle.resume();
	}

	if (m_handle.done()) {
		// log("Done");
		m_status = Status::Done;
		return {};
	}

	if (std::holds_alternative<detail::SpawnCommand>(promise.scheduler_command)
		|| std::holds_alternative<detail::SendCommand>(
			promise.scheduler_command
		)) {
		return std::move(promise.scheduler_command);
	}

	// Else, the Actor is waiting for a message.
	// log("Blocked");
	m_status = Status::Blocked;

	return std::move(promise.scheduler_command);
}

void Actor::append_msg(std::string msg)
{
	m_message_queue.emplace_back(msg);
}

void Actor::log(std::string_view message)
{
	std::cout << std::format(
		"{} [{} {}] {}\n",
		std::chrono::system_clock::now(),
		id(),
		m_name,
		message
	);
}

detail::SendCommand Actor::send(ActorId receiver, std::string msg)
{
	return detail::SendCommand{receiver, std::move(msg)};
}

detail::ReceiveAwaiter Actor::receive()
{
	return detail::ReceiveAwaiter{m_message_queue};
}

Actor::Coroutine detail::Promise::get_return_object()
{
	return Actor::Coroutine::from_promise(*this);
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
