#include "actor.hpp"

#include "stage.hpp"

#include <fmt/core.h>

#include <cstdlib>

namespace coactor {

Actor::Actor(Stage& stage, std::function<result<void>(Stage&, Actor&)> fun)
	: stage{stage}, fun{fun}
{
}

Actor::~Actor() { inbox.shutdown(stage).get(); }

result<int> Actor::receive()
{
	const auto tpe = stage.get_executor();
	return inbox.pop(tpe).run();
}

result<void> Actor::send(int i)
{
	const auto tpe = stage.get_executor();
	return inbox.push(tpe, i).run();
}

result<void> Actor::run() { return fun(stage, *this); }

} // namespace coactor

coactor::result<void> run(coactor::Stage& stage)
{
	auto consumer = stage.spawn_actor(
		[](coactor::Stage&, coactor::Actor& actor) -> coactor::result<void> {
			while (true) {
				const int number = co_await actor.receive();
				if (number == -1) {
					break;
				}

				fmt::println("{}", number);
			}
		}
	);

	auto producer = stage.spawn_actor(
		[&consumer](coactor::Stage&, coactor::Actor&) -> coactor::result<void> {
			for (int i = 0; i < 20; i++) {
				consumer.send(i);
			}

			// Signal done:
			consumer.send(-1);

			co_return;
		}
	);

	co_await producer.run();
	co_await consumer.run();
}

int main()
{
	coactor::Stage stage;
	run(stage).get();

	return EXIT_SUCCESS;
}
