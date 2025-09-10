#pragma once

#include "scheduler5.hpp"

#include <coroutine>
#include <exception>
#include <iostream>
#include <list>
#include <memory>
#include <string>

namespace coactor {

class Promise;
class Scheduler;

class Actor {
public:
	using Handle = std::coroutine_handle<Promise>;

	enum class Status {
		Ready,
		Running,
		Blocked,
		Terminated
	};

	virtual ~Actor();

	void init();
	void resume();

	template <typename ActorT, typename... Args>
	Actor::Handle spawn(Args...);

	bool is_done() const { return m_handle.done(); }

	Handle id() const { return m_handle; }

	Status status() const { return m_status; }

	void set_scheduler(Scheduler* scheduler) { m_scheduler = scheduler; }

	bool is_message_queue_empty() const;

	void append_msg(const std::string& msg);

private:
	virtual Handle act() = 0;

	class ReceiveAwaiter {
	public:
		ReceiveAwaiter(std::list<std::string>& message_queue)
			: m_message_queue{message_queue}
		{
		}

		bool await_ready() { return false; }

		bool await_suspend(std::coroutine_handle<>) { return true; }

		std::string await_resume()
		{
			std::string msg = m_message_queue.front();
			m_message_queue.pop_front();
			return msg;
		}

	private:
		std::list<std::string>& m_message_queue;
	};

	Handle m_handle{};

	Status m_status{Status::Ready};

	std::list<std::string> m_message_queue;

	Scheduler* m_scheduler;

protected:
	void send(Handle receiver, const std::string& msg);

	ReceiveAwaiter receive();
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

template <typename ActorT, typename... Args>
Actor::Handle Actor::spawn(Args... args)
{
	return m_scheduler->insert(std::make_unique<ActorT>(args...));
}

} // namespace coactor

template <typename... ArgTypes>
struct std::coroutine_traits<coactor::Actor::Handle, ArgTypes...> {
	using promise_type // NOLINT(readability-identifier-naming)
		= coactor::Promise;
};
