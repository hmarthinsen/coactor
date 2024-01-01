#pragma once

#include "coactor/concurrent_queue.hpp"
#include "coactor/message.hpp"

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
	Result<void> send(ActorId recipient, const Message& message);
	Result<Message> receive();

private:
	ConcurrentQueue m_inbox;
	bool m_is_done = false;
	Stage& m_stage;
	ActorId m_id;
};

} // namespace coactor
