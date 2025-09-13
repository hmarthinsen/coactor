#pragma once

#include "coactor/detail/utils.hpp"

#include <coroutine>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace coactor {

namespace detail {

class ReceiveAwaiter {
public:
	ReceiveAwaiter(std::list<std::string>& message_queue)
		: m_message_queue{message_queue}
	{
	}

	bool await_ready() { return !m_message_queue.empty(); }

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

class SpawnAwaiter {
public:
	SpawnAwaiter(ActorId id) : m_id{id} { }

	bool await_ready() { return false; }

	bool await_suspend(std::coroutine_handle<>) { return true; }

	ActorId await_resume() { return m_id; }

private:
	ActorId m_id;
};

class Promise;

} // namespace detail

class Scheduler;

class Actor {
public:
	enum class Status {
		Ready,
		Running,
		Blocked,
		Done
	};

	using Coroutine = std::coroutine_handle<detail::Promise>;
	virtual ~Actor();
	void init();

	void set_name(std::string_view name) { m_name = name; }

	std::string_view name() const { return m_name; }

	ActorId id() const { return ActorId(this); }

	detail::SchedulerCommand resume();

	Status status() const { return m_status; }

	void set_ready();

	void append_msg(std::string msg);

protected:
	template <typename ActorT, typename... Args>
	detail::SpawnCommand spawn(Args...);
	detail::SendCommand send(ActorId receiver, std::string msg);

	detail::ReceiveAwaiter receive();

	void log(std::string_view message);

private:
	virtual Coroutine act() = 0;

	Coroutine m_handle{};
	Status m_status{Status::Ready};
	std::list<std::string> m_message_queue{};

	std::string m_name{"Unknown"};
};

namespace detail {

class Promise {
public:
	Actor::Coroutine get_return_object();
	std::suspend_always initial_suspend();
	void return_void();
	void unhandled_exception();
	std::suspend_always final_suspend() noexcept;

	std::suspend_always yield_value(SendCommand cmd)
	{
		scheduler_command = cmd;
		return {};
	}

	SpawnAwaiter yield_value(SpawnCommand cmd)
	{
		ActorId id{cmd.actor.get()};
		scheduler_command = std::move(cmd);
		return SpawnAwaiter{id};
	}

	void reset() { scheduler_command = {}; }

	SchedulerCommand scheduler_command;
};

} // namespace detail

template <typename ActorT, typename... Args>
detail::SpawnCommand Actor::spawn(Args... args)
{
	auto actor = std::make_unique<ActorT>(args...);
	std::string_view name = detail::get_type_name<ActorT>();
	actor->set_name(name);

	// log(std::format("Spawning {}", name));

	return detail::SpawnCommand{std::move(actor)};
}

} // namespace coactor

template <typename... ArgTypes>
struct std::coroutine_traits<coactor::Actor::Coroutine, ArgTypes...> {
	using promise_type // NOLINT(readability-identifier-naming)
		= coactor::detail::Promise;
};
