#pragma once

#include "coactor/detail/actor_coro.hpp"
#include "coactor/detail/utils.hpp"
#include "coactor/runtime.hpp"

#include <chrono>
#include <coroutine>
#include <format>
#include <functional>
#include <list>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

namespace coactor {

// NOT thread-safe.
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

	void register_ready_callback(std::function<void()> ready_callback)
	{
		m_ready_callback = ready_callback;
	}

	void init(Runtime*, Address, std::string_view name);

	std::string_view name() const { return m_name; }

	Address address() const { return m_address; }

	Status status() const { return m_status; }

	std::optional<std::chrono::time_point<std::chrono::steady_clock>>
	timeout_point() const;
	std::string timeout_msg() const;

	void resume();

	// Thread-safe.
	void append_msg(std::string msg);

protected:
	template <typename ActorT, typename... Args>
	Address spawn(Args...);
	void send(Address receiver, const std::string& msg);
	void send(const std::string& receiver_name, const std::string& msg);
	detail::ReceiveAwaiter receive(
		std::optional<std::chrono::milliseconds> timeout = std::nullopt,
		const std::string& timeout_msg = ""
	);

	void log(std::string_view message);

private:
	virtual Coroutine act() = 0;

	std::string m_name{"Unknown"};

	Address m_address{0};

	Status m_status{Status::Ready};
	Coroutine m_coro_handle{};

	std::mutex m_message_queue_mutex{};
	std::list<std::string> m_message_queue{};

	Runtime* m_runtime;

	std::function<void()> m_ready_callback{};
};

template <typename ActorT, typename... Args>
Address Actor::spawn(Args... args)
{
	return m_runtime->spawn_actor<ActorT>(args...);
}

namespace detail {

class Promise {
public:
	std::coroutine_handle<Promise> get_return_object();
	std::suspend_always initial_suspend() { return {}; }
	void return_void() { }
	void unhandled_exception();
	std::suspend_always final_suspend() noexcept { return {}; }

	void reset_timeout()
	{
		timeout_point = std::nullopt;
		timeout_msg.clear();
	}

	Address address;
	std::string name;

	std::optional<std::chrono::time_point<std::chrono::steady_clock>>
		timeout_point;
	std::string timeout_msg;
};

} // namespace detail

} // namespace coactor

template <typename T>
struct std::
	formatter<T, std::enable_if_t<std::is_base_of_v<coactor::Actor, T>, char>>
	: std::formatter<std::string> {
	auto format(const T& actor, format_context& ctx) const
	{
		return formatter<std::string>::format(
			coactor::detail::colorize(
				std::format("{}:{}", actor.address(), actor.name()),
				actor.address()
			),
			ctx
		);
	}
};
