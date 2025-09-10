#pragma once

#include <coroutine>
#include <exception>
#include <format>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>

namespace coactor {

class Promise;
class Scheduler;

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

	template <typename A, typename... Args>
	Actor::Handle spawn(Args... args);

	Handle id() const
	{
		std::cout << std::format("Actor address: {}\n", m_handle.address());
		return m_handle;
	}

	void set_scheduler(Scheduler* scheduler) { m_scheduler = scheduler; }

	void append_msg(const std::string& msg) { m_message_queue.push_back(msg); }

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

	std::list<std::string> m_message_queue;

	Scheduler* m_scheduler;

protected:
	void send(Handle receiver, const std::string& msg);

	ReceiveAwaiter receive() { return {m_message_queue}; }
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

class Scheduler {
public:
	template <typename A, typename... Args>
	A::Handle spawn(Args... args);
	void send(Actor::Handle receiver, const std::string& msg);

private:
	std::map<Actor::Handle, std::unique_ptr<Actor>> m_actors;
};

class Promise;

template <typename A, typename... Args>
A::Handle Scheduler::spawn(Args... args)
{
	auto actor = std::make_unique<A>(args...);
	actor->set_scheduler(this);
	actor->resume();

	const auto id = actor->id();
	m_actors[id] = std::move(actor);

	return id;
}

void Scheduler::send(Actor::Handle receiver, const std::string& msg)
{
	m_actors[receiver]->append_msg(msg);
}

void Actor::send(Handle receiver, const std::string& msg)
{
	std::cout << std::format("Sending message: {}\n", msg);
	m_scheduler->send(receiver, msg);
}

template <typename A, typename... Args>
Actor::Handle Actor::spawn(Args... args)
{
	return m_scheduler->spawn<A>(args...);
}

} // namespace coactor

template <typename... ArgTypes>
struct std::coroutine_traits<coactor::Actor::Handle, ArgTypes...> {
	using promise_type // NOLINT(readability-identifier-naming)
		= coactor::Promise;
};
