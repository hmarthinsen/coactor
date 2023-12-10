#include "coactor.hpp"

#include <format>
#include <iostream>

using namespace coactor;

class Consumer : public Actor {
public:
	using Actor::Actor;

	Result<void> run() override
	{
		int num_producers = 1000;
		while (true) {
			const int number = co_await receive();
			if (number == -1) {
				num_producers--;

				if (num_producers == 0) {
					break;
				}
			}
		}
	}
};

class Producer : public Actor {
public:
	Producer(Stage& stage, ActorId id, ActorId consumer_id)
		: Actor(stage, id), m_consumer_id{consumer_id}
	{
	}

	Result<void> run() override
	{
		for (int i = 0; i < 1000; i++) {
			co_await send(m_consumer_id, i);
		}
		// Signal done:
		co_await send(m_consumer_id, -1);

		co_return;
	}

	ActorId m_consumer_id;
};

int main()
{
	Stage stage;

	auto consumer_id = stage.spawn_actor<Consumer>();
	for (int i = 0; i < 1000; i++) {
		stage.spawn_actor<Producer>(consumer_id);
	}

	stage.wait_until_done();
	return 0;
}
