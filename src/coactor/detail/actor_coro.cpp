#include "coactor/detail/actor_coro.hpp"

namespace coactor::detail {

bool ReceiveAwaiter::await_ready()
{
	return !m_message_queue.empty();
}

std::string ReceiveAwaiter::await_resume()
{
	std::string msg = m_message_queue.front();
	m_message_queue.pop_front();
	return msg;
}

} // namespace coactor::detail
