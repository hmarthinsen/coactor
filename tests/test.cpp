#include "coactor/coactor.hpp"

#include <format>
#include <string>

class Receiver : public coactor::Actor {
private:
	Coroutine act() override
	{
		while (true) {
			const std::string msg = co_await receive();
			log(std::format("Received: \"{}\"", msg));
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
		for (int i = 1; i <= 10; ++i) {
			const std::string msg = std::to_string(i);
			log(std::format("Sending to {}: \"{}\"", m_receiver, msg));
			send(m_receiver, msg);
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
		log(std::format("Spawned {}", receiver));

		const coactor::Address sender = spawn<Sender>(receiver);
		log(std::format("Spawned {}", sender));

		co_return;
	}
};

int main()
{
	return coactor::run<Main>();
}
