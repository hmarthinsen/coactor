#include "coactor.hpp"

#include <chrono>
#include <iostream>
#include <thread>

using namespace coactor;
using namespace std::chrono_literals;

class DatabaseServer : public Actor {
private:
	std::string run_query(const std::string& query_def)
	{
		// TODO: Actor-specific timer of 2 s here.
		std::this_thread::sleep_for(2s);
		return query_def + " result";
	}

public:
	using Actor::Actor;

	Result<void> run() override
	{
		// while (true) {
		const Message message = co_await receive();
		if (message[0] == "run_query") {
			ActorId caller = message[1];
			std::string query_def = message[2];
			Message reply{"query_result", run_query(query_def)};
			co_await send(caller, reply);
		} else {
			// TODO: Put message back in queue.
		}
		// }
	}
};

class MainActor : public Actor {
public:
	MainActor(Stage& stage, ActorId id, ActorId db_server_id)
		: Actor(stage, id), m_db_server_id{db_server_id}
	{
	}

	Result<void> run() override
	{
		Message query = {"run_query", get_actor_id(), "query 1"};
		co_await send(m_db_server_id, query);
		const Message result = co_await receive();
		if (result[0] == "query_result") {
			std::cout << result[1] << std::endl;
		} else {
			// TODO: Put message back in queue.
		}
	}

	ActorId m_db_server_id;
};

int main()
{
	Stage stage;

	ActorId db_server_id = stage.spawn_actor<DatabaseServer>();
	stage.spawn_actor<MainActor>(db_server_id);

	stage.wait_until_done();
	return 0;
}
