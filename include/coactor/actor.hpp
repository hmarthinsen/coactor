#pragma once

// #include "coactor/inbox.hpp"
// #include "coactor/stage.hpp"
#include "coactor/result.hpp"

#include <functional>

namespace coactor {

class Stage;
class Inbox;

using ActorId = int;
using Actor = std::function<Result<void>(Stage&, ActorId, Inbox&)>;

// class Stage;

// class Actor {
// 	friend Stage;

// public:
// 	Actor(Stage& stage, ActorId id);
// 	~Actor();

// 	virtual Result<void> run() = 0;
// 	ActorId get_actor_id();

// protected:
// 	Result<void> send(ActorId recipient, const Message& message);
// 	Result<Message> receive();

// private:
// 	ConcurrentQueue m_inbox;
// 	bool m_is_done = false;
// 	Stage& m_stage;
// 	ActorId m_id;
// };

} // namespace coactor
