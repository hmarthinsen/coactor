#include "actor3.hpp"

#include <format>
#include <iostream>
#include <string>

class Receiver : public coactor::Actor {
	Handle act() override
	{
		std::cout << "Starting Receiver actor\n";

		while (true) {
			const std::string msg = co_await receive();
			std::cout << std::format("Received message: {}\n", msg);
		}
	}
};

class Sender : public coactor::Actor {
public:
	Sender(Handle receiver) : m_receiver{receiver} { }

private:
	Handle act() override
	{
		std::cout << "Starting Sender actor\n";

		for (int i = 1; i <= 100; ++i) {
			send(m_receiver, std::to_string(i));
		}

		co_return;
	}

	Handle m_receiver;
};

class Main : public coactor::Actor {
	Handle act() override
	{
		Handle receiver = spawn<Receiver>();
		std::cout << "Receiver spawned.\n";

		spawn<Sender>(receiver);
		std::cout << "Sender spawned.\n";

		std::cout << "Resuming Receiver again.\n";
		receiver.resume();
		receiver.resume();
		receiver.resume();

		co_return;
	}
};

int main()
{
	coactor::Scheduler scheduler;

	scheduler.spawn<Main>();

	std::cout << "Finished.\n";
	return 0;
}
