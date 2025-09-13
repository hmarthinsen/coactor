#include "coactor/actor.hpp"

#include "coactor/detail/utils.hpp"
#include "coactor/scheduler.hpp"

#include <format>
#include <variant>

namespace coactor {

Actor::~Actor()
{
	log("Destroyed");
	m_coro_handle.destroy();
}

void Actor::init()
{
	m_coro_handle = act();
	log("Created");
}

void Actor::set_ready()
{
	log("Status Ready");
	m_status = Status::Ready;
}

detail::SchedulerCommand Actor::resume()
{
	log("Status Running");
	m_status = Status::Running;

	auto& promise = m_coro_handle.promise();
	promise.reset();

	if (!m_coro_handle.done()) {
		m_coro_handle.resume();
	}

	if (m_coro_handle.done()) {
		log("Status Done");
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
	log("Status Blocked");
	m_status = Status::Blocked;

	return std::move(promise.scheduler_command);
}

void Actor::append_msg(std::string msg)
{
	m_message_queue.emplace_back(msg);
}

detail::SendCommand Actor::send(ActorId receiver, std::string msg)
{
	return detail::SendCommand{receiver, std::move(msg)};
}

detail::ReceiveAwaiter Actor::receive()
{
	return detail::ReceiveAwaiter{m_message_queue};
}

void Actor::log(std::string_view message)
{
	detail::log(std::format("{}:{}", id(), m_name), message);
}

namespace detail {

ActorCoroutine detail::Promise::get_return_object()
{
	return ActorCoroutine::from_promise(*this);
}

void detail::Promise::unhandled_exception()
{
	try {
		std::rethrow_exception(std::current_exception());
	} catch (const std::exception& e) {
		detail::log("System", std::format("Unhandled exception: {}", e.what()));
	} catch (...) {
		detail::log("System", "Unhandled exception: Unknown error");
	}
}

std::suspend_always Promise::yield_value(SendCommand cmd)
{
	scheduler_command = cmd;
	return {};
}

SpawnAwaiter Promise::yield_value(SpawnCommand cmd)
{
	ActorId id = cmd.actor->id();
	scheduler_command = std::move(cmd);
	return SpawnAwaiter{id};
}

} // namespace detail

} // namespace coactor
