#include "coactor.hpp"

#include <iostream>
#include <syncstream>
#include <vector>

#include <cstddef>

using namespace coactor;

class Consumer : public Actor {
public:
	using Actor::Actor;

	Result<void> run() override
	{
		int num_producers = 2;
		while (true) {
			std::osyncstream(std::cout)
				<< get_actor_id() << ": receiving" << std::endl;
			const std::vector<int> array = co_await receive();
			std::osyncstream(std::cout)
				<< get_actor_id() << ": received data" << std::endl;

			long long sum = 0;
			for (std::size_t i = 0; i < array.size(); i++) {
				sum += array[i];
			}

			if (array.size() == 1) {
				num_producers--;

				std::osyncstream(std::cout)
					<< get_actor_id() << ": producers left: " << num_producers
					<< std::endl;

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
		std::vector<int> data;
		for (int i = 0; i < 8; i++) {
			data.push_back(i);
		}
		Message msg = data;

		for (int i = 0; i < 3; i++) {
			std::osyncstream(std::cout)
				<< get_actor_id() << ": sending " << i << std::endl;
			co_await send(m_consumer_id, msg);
		}
		// Signal done:
		std::osyncstream(std::cout)
			<< get_actor_id() << ": sending done" << std::endl;
		Message msg2 = std::vector<int>{-1};
		co_await send(m_consumer_id, msg2);

		co_return;
	}

	ActorId m_consumer_id;
};

int main()
{
	Stage stage;

	auto consumer_id1 = stage.spawn_actor<Consumer>();
	auto consumer_id2 = stage.spawn_actor<Consumer>();
	for (int i = 0; i < 2; i++) {
		stage.spawn_actor<Producer>(consumer_id1);
		stage.spawn_actor<Producer>(consumer_id2);
	}

	stage.wait_until_done();
	return 0;
}
