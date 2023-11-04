#include <asio.hpp>
#include <fmt/core.h>

#include <chrono>
#include <list>
#include <string>
#include <utility>

#include <cstdlib>

using namespace std::chrono_literals;

class Actor {
public:
	explicit Actor(asio::io_context& io) : m_io{io} { }

	virtual ~Actor() = default;

	void start() { asio::co_spawn(m_io, process_inbox(), asio::detached); }

	void post_message(std::string message)
	{
		m_inbox.push_back(std::move(message));
	}

private:
	asio::awaitable<void> process_inbox()
	{
		for (auto it = m_inbox.begin(); it != m_inbox.end();) {
			auto message = *it;
			bool was_handled = co_await handle_message(std::move(message));
			if (was_handled) {
				it = m_inbox.erase(it);
			} else {
				it++;
			}
		}
	}

	virtual asio::awaitable<bool> handle_message(std::string message)
	{
		co_return true;
	}

	asio::io_context& m_io;
	std::list<std::string> m_inbox;
};

// class PingPong : public Actor {
// public:
// 	explicit PingPong(asio::io_context& io) : Actor(io){};

// private:
// 	asio::awaitable<bool> handle_message(std::string message) override
// 	{
// 		co_await on_ping(std::move(message));
		
// 		co_return true;
// 	}

// 	asio::awaitable<void> on_ping(std::string message)
// 	{
// 		fmt::println("CSMS received: {}", message);

// 		co_await to_heartbeat_sender.async_write_some(
// 			asio::buffer("OK"), asio::use_awaitable
// 		);
// 	}
// };

// asio::awaitable<void> send_heartbeats(
// 	asio::io_context& io,
// 	asio::readable_pipe& from_csms,
// 	asio::writable_pipe& to_csms
// )
// {
// 	while (true) {
// 		asio::steady_timer timer{io, 3s};
// 		co_await timer.async_wait(asio::use_awaitable);

// 		co_await to_csms.async_write_some(
// 			asio::buffer("Heartbeat"), asio::use_awaitable
// 		);

// 		std::array<char, 32> message;
// 		co_await from_csms.async_read_some(
// 			asio::buffer(message), asio::use_awaitable
// 		);
// 		fmt::println(
// 			"Heartbeat sender received: {}",
// 			std::string{begin(message), end(message)}
// 		);
// 	}
// }

// asio::awaitable<void> fake_csms(
// 	const asio::io_context&,
// 	asio::readable_pipe& inbox,
// 	asio::writable_pipe& to_heartbeat_sender
// )
// {
// 	while (true) {
// 		std::array<char, 32> message;
// 		co_await inbox.async_read_some(
// 			asio::buffer(message), asio::use_awaitable
// 		);

// 		std::string string{begin(message), end(message)};

// 		co_await on_heartbeat(string);

// 		co_await to_heartbeat_sender.async_write_some(
// 			asio::buffer("OK"), asio::use_awaitable
// 		);
// 	}
// }

// Ping pong example
int main()
{
	asio::io_context io;

	// asio::readable_pipe from_heartbeat_sender(io);
	// asio::writable_pipe to_csms(io);
	// asio::connect_pipe(from_heartbeat_sender, to_csms);

	// FakeCsms fake_csms(io, from_heartbeat_sender);

	// asio::readable_pipe from_csms(io);
	// asio::writable_pipe to_heartbeat_sender(io);
	// asio::connect_pipe(from_csms, to_heartbeat_sender);

	// asio::co_spawn(
	// 	io,
	// 	fake_csms(io, from_heartbeat_sender, to_heartbeat_sender),
	// 	asio::detached
	// );
	// asio::co_spawn(io, send_heartbeats(io, from_csms, to_csms), asio::detached);

	io.run();

	return EXIT_SUCCESS;
}
