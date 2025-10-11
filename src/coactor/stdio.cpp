#include "coactor/stdio.hpp"

#include <iostream>

namespace coactor {

Actor::Coroutine StdIo::act()
{
	const std::string input_cmd = "in";
	const std::string output_cmd = "out";

	while (true) {
		std::string msg = co_await receive();

		if (msg.starts_with(input_cmd)) {
			msg = msg.substr(input_cmd.size());

			std::size_t next_pos;
			Address reply_addr = std::stoul(msg, &next_pos);

			std::string prompt = msg.substr(next_pos + 1);
			std::cout << prompt + " ";

			std::string input;
			std::cin >> input;

			send(reply_addr, input);
		} else if (msg.starts_with(output_cmd)) {
			msg = msg.substr(output_cmd.size() + 1);
			std::cout << msg;
		} else {
			log("Invalid command: " + msg);
		}
	}

	co_return;
}

} // namespace coactor
