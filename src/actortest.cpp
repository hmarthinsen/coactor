#include "coactor.hpp"

#include <iostream>

using namespace coactor;

class Consumer : public Actor {
public:
	using Actor::Actor;

private:
	Result<void> run() override
	{
		while (true) {
			const int number = co_await receive();
			if (number == -1) {
				break;
			}

			std::cout << number << "\n";
		}
	}
};

class Producer : public Actor {
public:
	Producer(Stage& stage, ActorId consumer_id)
		: Actor(stage), m_consumer_id{consumer_id}
	{
	}

private:
	Result<void> run() override
	{
		for (int i = 0; i < 20; i++) {
			send(m_consumer_id, i);
		}
		// Signal done:
		send(m_consumer_id, -1);

		co_return;
	}

	ActorId m_consumer_id;
};

int main()
{
	Stage stage;

	auto consumer_id = stage.spawn_actor<Consumer>();
	stage.spawn_actor<Producer>(consumer_id);

	stage.run().get();

	return 0;
}
