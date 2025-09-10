#include "coactor.hpp"

#include <iostream>
#include <syncstream>
#include <vector>

#include <cstddef>

using namespace coactor;

Result<void> consumer(Stage&, ActorId actor_id, Inbox& inbox)
{
	int num_producers = 2;
	while (true) {
		std::osyncstream(std::cout) << actor_id << ": receiving" << std::endl;
		const std::vector<int> array = co_await inbox.receive();
		std::osyncstream(std::cout)
			<< actor_id << ": received data" << std::endl;

		long long sum = 0;
		for (std::size_t i = 0; i < array.size(); i++) {
			sum += array[i];
		}

		if (array.size() == 1) {
			num_producers--;

			std::osyncstream(std::cout)
				<< actor_id << ": producers left: " << num_producers
				<< std::endl;

			if (num_producers == 0) {
				break;
			}
		}
	}
}

Result<void>
producer(Stage& stage, ActorId actor_id, Inbox&, ActorId consumer_id)
{
	std::vector<int> data;
	for (int i = 0; i < 8; i++) {
		data.push_back(i);
	}
	Message msg = data;

	for (int i = 0; i < 3; i++) {
		std::osyncstream(std::cout)
			<< actor_id << ": sending " << i << std::endl;
		co_await stage.send(consumer_id, msg);
	}
	// Signal done:
	std::osyncstream(std::cout) << actor_id << ": sending done" << std::endl;
	Message msg2 = std::vector<int>{-1};
	co_await stage.send(consumer_id, msg2);

	co_return;
}

int main()
{
	Stage stage;

	auto consumer_id1 = stage.spawn_actor(consumer);
	auto consumer_id2 = stage.spawn_actor(consumer);
	for (int i = 0; i < 2; i++) {
		stage.spawn_actor(
			[consumer_id1](Stage& stage, ActorId actor_id, Inbox& inbox) {
				return producer(stage, actor_id, inbox, consumer_id1);
			}
		);
		stage.spawn_actor(
			[consumer_id2](Stage& stage, ActorId actor_id, Inbox& inbox) {
				return producer(stage, actor_id, inbox, consumer_id2);
			}
		);
	}

	stage.wait_until_done();
	return 0;
}
