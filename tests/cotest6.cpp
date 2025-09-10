#include "coactor/actor.hpp"
#include "coactor/coactor.hpp"
#include "coactor/scheduler.hpp"

#include <format>
#include <iostream>
#include <string>

class Receiver : public coactor::Actor {
public:
	Receiver() { set_name("Receiver"); }

private:
	Handle act() override
	{
		while (true) {
			const std::string msg = co_await receive();
			std::cout << std::format("Received message: {}\n", msg);
		}
	}
};

class Sender : public coactor::Actor {
public:
	Sender(coactor::ActorId receiver) : m_receiver{receiver}
	{
		set_name("Sender");
	}

private:
	Handle act() override
	{
		for (int i = 1; i <= 10; ++i) {
			co_yield send(m_receiver, std::to_string(i));
			std::cout << std::format("Sent message: {}\n", i);
		}
		co_return;
	}

	coactor::ActorId m_receiver;
};

class Main : public coactor::Actor {
public:
	Main() { set_name("Main"); }

private:
	Handle act() override
	{
		coactor::ActorId receiver = co_yield spawn<Receiver>();
		co_yield spawn<Sender>(receiver);
		co_return;
	}
};

int main()
{
	coactor::run<Main>();

	return 0;
}
