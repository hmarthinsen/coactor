#pragma once

#include "concurrent_queue.hpp"

#include <functional>

namespace coactor {

using ActorId = int;

class Stage;

class Actor {
	friend Stage;

public:
	Actor(Stage& stage);
	~Actor();

protected:
	Result<void> send(ActorId recipient, int i);
	Result<int> receive();

private:
	virtual Result<void> run() = 0;

	Stage& m_stage;
	ConcurrentQueue m_inbox;
};

} // namespace coactor
