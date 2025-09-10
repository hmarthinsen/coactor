#include "actor5.hpp"
#include "coactor5.hpp"
#include "macros5.hpp"

#include <format>
#include <iostream>
#include <string>

ACTOR(
	Foo,
	while (true) {
		const std::string msg = RECEIVE();
		std::cout << std::format("Received message: {}\n", msg);

		if (msg == "100") {
			break;
		}
	}
);

ACTOR_BEGIN(Bar)
	while (true) {
		const std::string msg = RECEIVE();
		std::cout << std::format("Received message: {}\n", msg);

		if (msg == "100") {
			break;
		}
	}
ACTOR_END

class Receiver : public coactor::Actor {
	Handle act() override
	{
		while (true) {
			const std::string msg = co_await receive();
			std::cout << std::format("Received message: {}\n", msg);

			if (msg == "100") {
				break;
			}
		}
	}
};

class Sender : public coactor::Actor {
public:
	Sender(Handle receiver) : m_receiver{receiver} { }

private:
	Handle act() override
	{
		for (int i = 1; i <= 100; ++i) {
			send(m_receiver, std::to_string(i));
			std::cout << std::format("Sent message: {}\n", i);
		}
		co_return;
	}

	Handle m_receiver;
};

class Main : public coactor::Actor {
	Handle act() override
	{
		Handle receiver = spawn<Receiver>();
		spawn<Sender>(receiver);
		co_return;
	}
};

int main()
{
	coactor::run<Main>();
	return 0;
}
