#include "coactor/coactor.hpp"

#include <format>
#include <string>

using namespace std::chrono_literals;

class Receiver : public coactor::Actor {
private:
	Coroutine act() override
	{
		while (true) {
			const std::string msg = co_await receive();
			if (msg == "q") {
				break;
			}

			send("StdIo", std::format("out Received {}\n", msg));
		}

		co_return;
	}
};

class Sender : public coactor::Actor {
public:
	Sender(coactor::Address receiver) : m_receiver{receiver} { }

private:
	Coroutine act() override
	{
		while (true) {
			send("StdIo", std::format("in {} q to quit>", address()));
			const std::string msg = co_await receive();
			send(m_receiver, msg);

			if (msg == "q") {
				break;
			}
		}

		co_return;
	}

	coactor::Address m_receiver;
};

class Main : public coactor::Actor {
private:
	Coroutine act() override
	{
		const coactor::Address receiver = spawn<Receiver>();
		spawn<Sender>(receiver);

		co_return;
	}
};

int main()
{
	return coactor::run<Main>();
}
