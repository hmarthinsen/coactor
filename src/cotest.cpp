// Promise object,
// manipulated from inside the coroutine. The coroutine submits its result
// or exception through this object.

// Coroutine handle,
// manipulated from outside the coroutine. This is a non-owning handle used to
// resume execution of the coroutine or to destroy the coroutine frame.

// Coroutine state,
// which is internal, dynamically-allocated storage (unless the allocation is
// optimized out), object that contains
// - the promise object
// - the parameters (all copied by value)
// - some representation of the current suspension point, so that a resume knows
//   where to continue, and a destroy knows what local variables were in scope
// - local variables and temporaries whose lifetime spans the current suspension
//   point.

// When a coroutine begins execution, it performs the following:
// - allocates the coroutine state object using operator new.
// - copies all function parameters to the coroutine state: by-value parameters
//   are moved or copied, by-reference parameters remain references (thus, may
//   become dangling, if the coroutine is resumed after the lifetime of referred
//   object ends — see below for examples).
// - calls the constructor for the promise object. If the promise type has a
//   constructor that takes all coroutine parameters, that constructor is
//   called, with post-copy coroutine arguments. Otherwise the default
//   constructor is called.
// - calls promise.get_return_object() and keeps the result in a local variable.
//   The result of that call will be returned to the caller when the coroutine
//   first suspends. Any exceptions thrown up to and including this step
//   propagate back to the caller, not placed in the promise.
// - calls promise.initial_suspend() and co_awaits its result. Typical Promise
//   types either return a std::suspend_always, for lazily-started coroutines,
//   or std::suspend_never, for eagerly-started coroutines.
// - when co_await promise.initial_suspend() resumes, starts executing the body
//   of the coroutine.

#include <concepts>
#include <coroutine>
#include <exception>
#include <iostream>

struct ReturnObject {
	struct promise_type {
		ReturnObject get_return_object() { return {}; }

		std::suspend_never initial_suspend() { return {}; }

		std::suspend_never final_suspend() noexcept { return {}; }

		void unhandled_exception() { }
	};
};

struct Awaiter {
	std::coroutine_handle<>* hp_;

	constexpr bool await_ready() const noexcept { return false; }

	void await_suspend(std::coroutine_handle<> h) { *hp_ = h; }

	constexpr void await_resume() const noexcept { }
};

ReturnObject counter(std::coroutine_handle<>* continuation_out)
{
	Awaiter a{continuation_out};
	for (unsigned i = 0;; ++i) {
		co_await a;
		std::cout << "counter: " << i << std::endl;
	}
}

void main1()
{
	std::coroutine_handle<> h;
	counter(&h);
	for (int i = 0; i < 3; ++i) {
		std::cout << "In main1 function\n";
		h();
	}
	h.destroy();
}

int main()
{
	main1();
	return 0;
}
