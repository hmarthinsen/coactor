#pragma once

// IWYU pragma: private

#include <coroutine>
#include <list>
#include <mutex>
#include <string>

namespace coactor {

namespace detail {

class Promise;
using ActorCoroutine = std::coroutine_handle<Promise>;

class ReceiveAwaiter {
public:
	ReceiveAwaiter(
		std::list<std::string>& message_queue,
		std::mutex& message_queue_mutex
	)
		: m_message_queue{message_queue}
		, m_message_queue_mutex{message_queue_mutex}
	{
	}

	bool await_ready();
	bool await_suspend(std::coroutine_handle<>) { return true; }
	std::string await_resume();

private:
	std::list<std::string>& m_message_queue;
	std::mutex& m_message_queue_mutex;
};

} // namespace detail

} // namespace coactor

template <typename... Args>
struct std::coroutine_traits<coactor::detail::ActorCoroutine, Args...> {
	using promise_type // NOLINT(readability-identifier-naming)
		= coactor::detail::Promise;
};
