#pragma once

#include <coroutine>
#include <list>
#include <string>

#include <cstdint>

namespace coactor {

class Actor;

using Address = std::uint64_t;

namespace detail {

class Promise;
using ActorCoroutine = std::coroutine_handle<Promise>;

class ReceiveAwaiter {
public:
	ReceiveAwaiter(std::list<std::string>& message_queue)
		: m_message_queue{message_queue}
	{
	}

	bool await_ready();
	bool await_suspend(std::coroutine_handle<>) { return true; }
	std::string await_resume();

private:
	std::list<std::string>& m_message_queue;
};

class SpawnAwaiter {
public:
	SpawnAwaiter(Address address) : m_address{address} { }

	bool await_ready() { return false; }
	bool await_suspend(std::coroutine_handle<>) { return true; }
	Address await_resume() { return m_address; }

private:
	Address m_address;
};

} // namespace detail

} // namespace coactor

template <typename... Args>
struct std::coroutine_traits<coactor::detail::ActorCoroutine, Args...> {
	using promise_type // NOLINT(readability-identifier-naming)
		= coactor::detail::Promise;
};
