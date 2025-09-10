#include "cotest2.hpp"

#include <format>
#include <iostream>
#include <string>

class MyActor : public coactor::Actor {
	Handle act() override
	{
		std::cout << "In MyActor coroutine\n";

		send("Hello, World!");

		std::string msg = co_await receive();

		std::cout << std::format("Received message: {}\n", msg);
	}
};

int main()
{
	MyActor t;
	std::cout << "MyActor constructed. Now resuming it.\n";
	t.resume();

	std::cout << "Resuming MyActor again.\n";
	t.resume();

	std::cout << "Finished.\n";
	return 0;
}
