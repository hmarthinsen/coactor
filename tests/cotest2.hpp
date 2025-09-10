#pragma once

#include <coroutine>
#include <exception>
#include <format>
#include <iostream>
#include <string>

namespace coactor {

std::string g_msg{};

class Promise;

class Actor {
public:
	using Handle = std::coroutine_handle<Promise>;

	virtual ~Actor() { m_handle.destroy(); }

	void resume()
	{
		if (!m_handle) {
			m_handle = act();
		}

		if (!m_handle.done()) {
			m_handle.resume();
		}
	}

private:
	virtual Handle act() = 0;

	class ReceiveAwaiter {
	public:
		bool await_ready() { return false; }

		bool await_suspend(std::coroutine_handle<>) { return true; }

		std::string await_resume() { return g_msg; }
	};

	Handle m_handle{};

protected:
	void send(const std::string& msg)
	{
		std::cout << std::format("Sending message: {}\n", msg);

		g_msg = msg;
	}

	ReceiveAwaiter receive() { return {}; }
};

class Promise {
public:
	Actor::Handle get_return_object()
	{
		return Actor::Handle::from_promise(*this);
	}

	std::suspend_always initial_suspend() { return {}; }

	void return_void() { }

	void unhandled_exception()
	{
		try {
			std::rethrow_exception(std::current_exception());
		} catch (const std::exception& e) {
			std::cout << "Unhandled exception: " << e.what() << '\n';
		} catch (...) {
			std::cout << "Unhandled exception: Unknown error\n";
		}
	}

	std::suspend_always final_suspend() noexcept { return {}; }
};

} // namespace coactor

template <typename... ArgTypes>
struct std::coroutine_traits<coactor::Actor::Handle, ArgTypes...> {
	using promise_type // NOLINT(readability-identifier-naming)
		= coactor::Promise;
};
