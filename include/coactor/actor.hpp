#pragma once

#include "coactor/detail/actor_coro.hpp"
#include "coactor/detail/scheduler_cmd.hpp"
#include "coactor/detail/utils.hpp"

#include <format>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace coactor {

class Actor {
public:
	enum class Status {
		Ready,
		Running,
		Blocked,
		Done
	};

	using Coroutine = detail::ActorCoroutine;

	virtual ~Actor();

	void init();

	std::string_view name() const { return m_name; }
	void set_name(std::string_view name) { m_name = name; }

	ActorId id() const { return m_id; }
	void set_id(ActorId id) { m_id = id; }

	Status status() const { return m_status; }
	void set_ready();

	detail::SchedulerCommand resume();

	void append_msg(std::string msg);

protected:
	template <typename ActorT, typename... Args>
	detail::SpawnCommand spawn(Args...);
	detail::SendCommand send(ActorId receiver, std::string msg);
	detail::ReceiveAwaiter receive();

	void log(std::string_view message);

private:
	virtual Coroutine act() = 0;

	ActorId m_id{0};
	std::string m_name{"Unknown"};
	Status m_status{Status::Ready};
	Coroutine m_coro_handle{};
	std::list<std::string> m_message_queue{};
};

template <typename ActorT, typename... Args>
detail::SpawnCommand Actor::spawn(Args... args)
{
	const std::string_view actor_type_name = detail::get_type_name<ActorT>();

	auto actor = std::make_unique<ActorT>(args...);
	ActorId id = detail::get_unique_id();
	actor->set_id(id);
	actor->set_name(actor_type_name);

	log(std::format("Spawning {}:{}", id, actor_type_name));

	return detail::SpawnCommand{std::move(actor)};
}

namespace detail {

class Promise {
public:
	std::coroutine_handle<Promise> get_return_object();
	std::suspend_always initial_suspend() { return {}; }
	void return_void() { }
	void unhandled_exception();
	std::suspend_always final_suspend() noexcept { return {}; }

	std::suspend_always yield_value(SendCommand cmd);
	SpawnAwaiter yield_value(SpawnCommand cmd);

	void reset() { scheduler_command = {}; }

	SchedulerCommand scheduler_command;
};

} // namespace detail

} // namespace coactor
