#pragma once

#include "concurrent_queue.hpp"

#include <functional>

namespace coactor {

using ActorId = int;

class Stage;

class Actor {
	friend Stage;

public:
	Actor(Stage& stage, ActorId id);
	~Actor();

	virtual Result<void> run() = 0;
	ActorId get_actor_id();

protected:
	Result<void> send(ActorId recipient, int i);
	Result<int> receive();

private:
	ConcurrentQueue m_inbox;
	bool m_is_done = false;
	Stage& m_stage;
	ActorId m_id;
};

} // namespace coactor
