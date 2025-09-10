#pragma once

#include <format>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

namespace coactor {

class Promise;
class Actor;

/**
 * @brief A unique ID for an actor.
 */
struct ActorId {
	void* actor_address;

	// Necessary to be able to use ActorId as the key in a map.
	auto operator<=>(const ActorId&) const = default;
};

class Scheduler {
public:
	ActorId insert_actor(std::unique_ptr<Actor> actor);

	void run();

	// Send from other schedulers. Thread-safe.
	void transfer(ActorId receiver, const std::string& msg);

	void* id() { return this; }

private:
	void send(ActorId receiver, const std::string& msg);

	std::map<ActorId, std::unique_ptr<Actor>> m_actors;
	std::list<ActorId> m_ready_queue;

	std::mutex m_transfer_queue_mutex;
	std::list<std::pair<ActorId, std::string>> m_transfer_queue{};
};

} // namespace coactor

// Make ActorId usable with std::format.
template <>
struct std::formatter<coactor::ActorId> : std::formatter<std::string> {
	auto format(coactor::ActorId id, std::format_context& ctx) const
	{
		return std::formatter<std::string>::format(
			std::format("{}", id.actor_address),
			ctx
		);
	}
};
