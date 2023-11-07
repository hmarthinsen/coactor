#include "coactor.hpp"

#include <fmt/core.h>

#include <cstdlib>

using namespace coactor;

Result<void> consumer_fun(Actor& consumer)
{
	while (true) {
		const int number = co_await consumer.receive();
		if (number == -1) {
			break;
		}

		fmt::println("{}", number);
	}
}

Result<void> producer_fun(Actor& consumer)
{
	for (int i = 0; i < 20; i++) {
		consumer.send(i);
	}
	// Signal done:
	consumer.send(-1);

	co_return;
}

Result<void> run(Stage& stage)
{
	auto consumer = stage.spawn_actor([](Stage&, Actor& actor) -> Result<void> {
		return consumer_fun(actor);
	});

	auto producer
		= stage.spawn_actor([&consumer](Stage&, Actor&) -> Result<void> {
			  return producer_fun(consumer);
		  });

	co_await producer.run();
	co_await consumer.run();
}

int main()
{
	Stage stage;
	run(stage).get();

	return EXIT_SUCCESS;
}