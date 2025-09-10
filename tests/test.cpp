#include <chrono>
#include <coroutine>
#include <format>
#include <iostream>
#include <optional>

using namespace std::literals::chrono_literals;

namespace coactor {

class Actor {
public:
	using Id = int; // TODO: Use an appropriate type.

	explicit Actor(Id id) : m_id{id} { }

	Id id() const { return m_id; }

	// The promise_type instructs the std::coroutine library how to behave and
	// shouldn't contain user data.
	class promise_type { // NOLINT(readability-identifier-naming)
	public:
		Actor get_return_object(Id id) { return Actor{id}; }

		std::suspend_never initial_suspend() { return {}; }

		void return_void() { }

		void unhandled_exception() { }

		std::suspend_never final_suspend() noexcept { return {}; }
	};

private:
	Id m_id;
};

class Runtime {
public:
	void start(Actor actor) { actor.start(); }

private:
};

class Message { };

} // namespace coactor

coactor::Actor countdown(int counter, coactor::Actor::Id receiver)
{
	while (counter != 0) {
		const Message msg{counter};
		coactor::send(receiver, msg);
		--counter;
	}

	co_return;
}

coactor::Actor printer()
{
	for (int i = 0;; ++i) {
		const auto timeout = 5s;
		coactor::Message msg = co_await coactor::receive(timeout);

		std::cout << std::format("Received msg no. {}: {}\n", i, msg);

		if (msg == coactor::Message{"exit"}) {
			break;
		}
	}
}

// The main actor. Execution starts here.
coactor::Actor co_main()
{
	const coactor::Actor::Id printer_id = coactor::spawn(printer);
	const coactor::Actor::Id countdown_id
		= coactor::spawn(countdown, printer_id);

	co_return;
}

int main()
{
	coactor::Runtime runtime(co_main);
	runtime.start();

	return 0;
}
