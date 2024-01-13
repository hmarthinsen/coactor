#include "coactor.hpp"

#include <chrono>
#include <iostream>
#include <thread>

using namespace coactor;
using namespace std::chrono_literals;

Result<std::string> run_query(Stage& stage, const std::string& query_def)
{
	co_await stage.sleep(2s);

	co_return query_def + " result";
}

Result<void> database_server(Stage& stage, ActorId actor_id, Inbox& inbox)
{
	while (true) {
		const Message message
			= co_await inbox.receive([](const Message& message) {
				  return message.is_array() && !message.empty()
					  && message[0] == "run_query";
			  });

		const ActorId caller = message[1];
		const std::string query_def = message[2];
		const Message reply
			= {"query_result", co_await run_query(stage, query_def)};
		co_await stage.send(caller, reply);
	}
}

Result<void> main_actor(Stage& stage, ActorId actor_id, Inbox& inbox)
{
	const ActorId db_server_id = stage.spawn_actor(database_server);

	const Message query = {"run_query", actor_id, "query 1"};
	co_await stage.send(db_server_id, query);

	const Message result = co_await inbox.receive([](const Message& message) {
		return message.is_array() && !message.empty()
			&& message[0] == "query_result";
	});

	std::cout << result[1] << std::endl;
}

int main()
{
	Stage stage;
	stage.spawn_actor(main_actor);

	stage.wait_until_done();
	return 0;
}
