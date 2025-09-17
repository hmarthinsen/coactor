#include "coactor/actor.hpp"

#include "coactor/detail/actor_coro.hpp"
#include "coactor/detail/utils.hpp"
#include "coactor/scheduler.hpp"

#include <format>
#include <string>

namespace coactor {

Actor::~Actor()
{
	m_coro_handle.destroy();
}

void Actor::init()
{
	m_coro_handle = act();

	auto& promise = m_coro_handle.promise();
	promise.address = m_address;
	promise.name = m_name;

	m_is_initialized = true;
}

void Actor::set_name(std::string_view name)
{
	m_name = name;

	if (m_is_initialized) {
		auto& promise = m_coro_handle.promise();
		promise.name = name;
	}
}

void Actor::set_ready()
{
	m_status = Status::Ready;
}

detail::SchedulerCommand Actor::resume()
{
	m_status = Status::Running;

	auto& promise = m_coro_handle.promise();
	promise.reset();

	if (!m_coro_handle.done()) {
		m_coro_handle.resume();
	}

	if (m_coro_handle.done()) {
		log(detail::bold("Done"));
		m_status = Status::Done;
	} else {
		m_status = Status::Blocked;
	}

	return std::move(promise.scheduler_command);
}

void Actor::append_msg(std::string msg)
{
	m_message_queue.emplace_back(msg);
}

detail::SendCommand Actor::send(Address receiver, std::string msg)
{
	log(std::format(
		"Sending to {}: \"{}\"",
		detail::colorize(std::to_string(receiver), receiver),
		msg
	));
	return detail::SendCommand{address(), receiver, std::move(msg)};
}

detail::ReceiveAwaiter Actor::receive()
{
	return detail::ReceiveAwaiter{m_message_queue};
}

void Actor::log(std::string_view message)
{
	detail::log(std::format("{}", *this), message);
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
		detail::log(
			coactor::detail::colorize(
				std::format("{}:{}", address, name),
				address
			),
			detail::red(std::format("Unhandled exception: {}", e.what()))
		);
	} catch (...) {
		detail::log(
			coactor::detail::colorize(
				std::format("{}:{}", address, name),
				address
			),
			detail::red("Unhandled exception: Unknown error")
		);
	}
}

std::suspend_always Promise::yield_value(SendCommand cmd)
{
	scheduler_command = cmd;
	return {};
}

SpawnAwaiter Promise::yield_value(SpawnCommand cmd)
{
	Address address = cmd.actor->address();
	scheduler_command = std::move(cmd);
	return SpawnAwaiter{address};
}

} // namespace detail

} // namespace coactor
