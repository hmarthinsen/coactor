#include "coactor/inbox.hpp"

#include "coactor/stage.hpp"

namespace coactor {

Result<Message> Inbox::receive()
{
	const auto executor = m_stage.get_executor();
	co_return Message::from_msgpack(co_await m_queue.pop(executor));
}

} // namespace coactor
