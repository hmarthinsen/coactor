#include "coactor/actor.hpp"

#include "coactor/detail/actor_coro.hpp"
#include "coactor/detail/utils.hpp"

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

void Actor::set_ready()
{
	m_status = Status::Ready;
}

void Actor::resume()
{
	m_status = Status::Running;

	if (!m_coro_handle.done()) {
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
	m_message_queue.emplace_back(std::move(msg));
}

void Actor::send(Address receiver, const std::string& msg)
{
	m_runtime->send(receiver, msg);
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

} // namespace detail

} // namespace coactor
