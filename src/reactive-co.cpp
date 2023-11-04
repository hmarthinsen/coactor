#include <asio.hpp>
#include <fmt/core.h>

#include <array>
#include <chrono>
#include <exception>
#include <string>

#include <cstdlib>

using namespace std::chrono_literals;

asio::awaitable<void> send_heartbeats(
	asio::io_context& io,
	asio::readable_pipe& from_csms,
	asio::writable_pipe& to_csms
)
{
	while (true) {
		asio::steady_timer timer{io, 3s};
		co_await timer.async_wait(asio::use_awaitable);

		co_await to_csms.async_write_some(
			asio::buffer("Heartbeat"), asio::use_awaitable
		);

		std::array<char, 32> message;
		co_await from_csms.async_read_some(
			asio::buffer(message), asio::use_awaitable
		);
		fmt::println(
			"Heartbeat sender received: {}",
			std::string{begin(message), end(message)}
		);
	}
}

asio::awaitable<void> fake_csms(
	const asio::io_context&,
	asio::readable_pipe& from_heartbeat_sender,
	asio::writable_pipe& to_heartbeat_sender
)
{
	while (true) {
		std::array<char, 32> message;
		co_await from_heartbeat_sender.async_read_some(
			asio::buffer(message), asio::use_awaitable
		);
		fmt::println(
			"CSMS received: {}", std::string{begin(message), end(message)}
		);

		co_await to_heartbeat_sender.async_write_some(
			asio::buffer("OK"), asio::use_awaitable
		);
	}
}

// Heartbeat coroutine example
int main()
{
	try {
		asio::io_context io;

		asio::readable_pipe from_heartbeat_sender(io);
		asio::writable_pipe to_csms(io);
		asio::connect_pipe(from_heartbeat_sender, to_csms);

		asio::readable_pipe from_csms(io);
		asio::writable_pipe to_heartbeat_sender(io);
		asio::connect_pipe(from_csms, to_heartbeat_sender);

		asio::co_spawn(
			io,
			fake_csms(io, from_heartbeat_sender, to_heartbeat_sender),
			asio::detached
		);
		asio::co_spawn(
			io, send_heartbeats(io, from_csms, to_csms), asio::detached
		);

		io.run();
	} catch (std::exception& e) {
		fmt::println(stderr, "{}", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
