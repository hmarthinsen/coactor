#pragma once

#include "coactor/concurrent_queue.hpp"
#include "coactor/message.hpp"
#include "coactor/result.hpp"

namespace coactor {

class Stage;

class Inbox {
	friend Stage;

public:
	Inbox(Stage& stage) : m_stage{stage} { }

	Result<Message> receive();

private:
	ConcurrentQueue m_queue;
	Stage& m_stage;
};
} // namespace coactor
