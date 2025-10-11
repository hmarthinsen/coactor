#include "coactor/actor.hpp"

#include "coactor/detail/actor_coro.hpp"
#include "coactor/detail/utils.hpp"

#include <chrono>
#include <mutex>
#include <string>

namespace coactor {

Actor::~Actor()
{
	m_coro_handle.destroy();
}

void Actor::init(Runtime* runtime, Address address, std::string_view name)
{
	m_runtime = runtime;
	m_address = address;
	m_name = name;

	m_coro_handle = act();

	auto& promise = m_coro_handle.promise();
	promise.address = m_address;
	promise.name = m_name;
}

std::optional<std::chrono::time_point<std::chrono::steady_clock>>
Actor::timeout_point() const
{
	auto& promise = m_coro_handle.promise();
	return promise.timeout_point;
}

std::string Actor::timeout_msg() const
{
	auto& promise = m_coro_handle.promise();
	return promise.timeout_msg;
}

void Actor::resume()
{
	if (!m_coro_handle.done()) {
		m_status = Status::Running;
		m_coro_handle.promise().reset_timeout();
		m_coro_handle.resume();
	}

	if (m_coro_handle.done()) {
		m_status = Status::Done;
	} else {
		m_status = Status::Blocked;
	}
}

void Actor::append_msg(std::string msg)
{
	// FIXME: Status mutex?
	if (m_status == Status::Done) {
		return;
	}

	{
		std::lock_guard lock{m_message_queue_mutex};
		m_message_queue.emplace_back(std::move(msg));
	}

	if (m_status == Status::Blocked) {
		m_status = Status::Ready;
		m_ready_callback();
	}
}

void Actor::send(Address receiver, const std::string& msg)
{
	m_runtime->send(receiver, msg);
}

void Actor::send(const std::string& receiver_name, const std::string& msg)
{
	m_runtime->send(receiver_name, msg);
}

detail::ReceiveAwaiter Actor::receive(
	std::optional<std::chrono::milliseconds> timeout,
	const std::string& timeout_msg
)
{
	if (timeout.has_value()) {
		detail::Promise& promise = m_coro_handle.promise();
		promise.timeout_point = std::chrono::steady_clock::now() + *timeout;
		promise.timeout_msg = timeout_msg;
	}
	return detail::ReceiveAwaiter{m_message_queue, m_message_queue_mutex};
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

} // namespace detail

} // namespace coactor
