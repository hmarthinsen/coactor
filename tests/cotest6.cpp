#include "coactor/actor.hpp"
#include "coactor/coactor.hpp"

#include <format>
#include <string>

class Receiver : public coactor::Actor {
private:
	Coroutine act() override
	{
		while (true) {
			const std::string msg = co_await receive();
			log(std::format("Received message: {}", msg));
		}
	}
};

class Sender : public coactor::Actor {
public:
	Sender(coactor::ActorId receiver) : m_receiver{receiver} { }

private:
	Coroutine act() override
	{
		for (int i = 1; i <= 10; ++i) {
			log(std::format("Sending message: {}", i));
			co_yield send(m_receiver, std::to_string(i));
		}
	}

	coactor::ActorId m_receiver;
};

class Main : public coactor::Actor {
private:
	Coroutine act() override
	{
		coactor::ActorId receiver = co_yield spawn<Receiver>();
		co_yield spawn<Sender>(receiver);
	}
};

int main()
{
	coactor::run<Main>();
	return 0;
}
