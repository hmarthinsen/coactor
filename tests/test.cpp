#include "coactor/actor.hpp"
#include "coactor/runtime.hpp"

#include <string>

using namespace std::chrono_literals;

class Receiver : public coactor::Actor {
private:
	Coroutine act() override
	{
		while (true) {
			const std::string msg = co_await receive();
			log(std::format("Received: \"{}\"", msg));
		}
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
			co_yield send(m_receiver, msg);
		}
	}

	coactor::Address m_receiver;
};

class Main : public coactor::Actor {
private:
	Coroutine act() override
	{
		coactor::Address receiver = co_yield spawn<Receiver>();
		co_yield spawn<Sender>(receiver);
	}
};

int main()
{
	coactor::Runtime runtime;
	runtime.add_schedulers(4);
	runtime.spawn_actor<Main>();
	runtime.run();

	// TODO: Want to write this:
	// coactor::Runtime runtime{4}; // Default arg = num cores.
	// runtime.run<Main>(args...);

	return 0;
}
