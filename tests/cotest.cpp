#include <coroutine>
#include <exception>
#include <iostream>
#include <stdexcept>

class Task {
public:
	// The promise_type instructs the std::coroutine library how to behave
	// and shouldn't contain user data.
	class promise_type { // NOLINT(readability-identifier-naming)
	public:
		Task get_return_object()
		{
			auto handle
				= std::coroutine_handle<promise_type>::from_promise(*this);
			return Task{handle};
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

		std::suspend_never final_suspend() noexcept { return {}; }
	};

	Task(std::coroutine_handle<promise_type> handle) : m_handle{handle} { }

	void resume() const { m_handle.resume(); }

private:
	std::coroutine_handle<promise_type> m_handle;
};

Task task()
{
	std::cout << "In task\n";

	throw std::runtime_error("Runtime error in task!");

	std::cout << "After exception\n";

	co_return;
}

int main()
{
	Task t = task();
	std::cout << "Task constructed. Now resuming it.\n";

	t.resume();
	std::cout << "Task resumed.\n";

	return 0;
}
