#pragma once

#include "scheduler.hpp"

#include <coroutine>
#include <list>
#include <memory>
#include <string>

namespace coactor {

namespace detail {

struct Envelope {
	ActorId receiver;
	std::string msg;
};

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

	using Handle = std::coroutine_handle<detail::Promise>;
	virtual ~Actor();
	void init();

	// void set_scheduler(Scheduler* scheduler) { m_scheduler = scheduler; }

	ActorId id() const { return ActorId{.actor_address = m_handle.address()}; }

	std::optional<detail::Envelope> resume();

	Status status() const { return m_status; }

	void set_ready();

	void append_msg(const std::string& msg);

protected:
	template <typename ActorT, typename... Args>
	std::unique_ptr<Actor> spawn(Args...);

	detail::Envelope send(ActorId receiver, const std::string& msg);
	detail::ReceiveAwaiter receive();

	void set_name(const std::string& name) { m_name = name; }

private:
	virtual Handle act() = 0;

	void log(const std::string& message);

	Handle m_handle{};
	Status m_status{Status::Ready};
	std::list<std::string> m_message_queue{};

	// Scheduler* m_scheduler{nullptr};

	std::string m_name{"Unknown"};
};

namespace detail {

class Promise {
public:
	Actor::Handle get_return_object();
	std::suspend_always initial_suspend();
	void return_void();
	void unhandled_exception();
	std::suspend_always final_suspend() noexcept;

	std::suspend_always yield_value(Envelope envelope)
	{
		yielded_envelope = envelope;
		return {};
	}

	SpawnAwaiter yield_value(std::unique_ptr<Actor> actor)
	{
		yielded_actor = std::move(actor);
		ActorId id{.actor_address = yielded_actor.get()};
		return {id};
	}

	std::optional<Envelope> yielded_envelope;
	std::unique_ptr<Actor> yielded_actor;
};

} // namespace detail

// template <typename ActorT, typename... Args>
// ActorId Actor::spawn(Args... args)
// {
// 	return m_scheduler->insert_actor(std::make_unique<ActorT>(args...));
// }

template <typename ActorT, typename... Args>
std::unique_ptr<Actor> Actor::spawn(Args... args)
{
	return std::make_unique<ActorT>(args...);
}

} // namespace coactor

template <typename... ArgTypes>
struct std::coroutine_traits<coactor::Actor::Handle, ArgTypes...> {
	using promise_type // NOLINT(readability-identifier-naming)
		= coactor::detail::Promise;
};
