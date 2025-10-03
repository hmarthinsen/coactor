#include "coactor/detail/actor_coro.hpp"

#include <mutex>

namespace coactor::detail {

bool ReceiveAwaiter::await_ready()
{
	std::lock_guard lock{m_message_queue_mutex};
	return !m_message_queue.empty();
}

std::string ReceiveAwaiter::await_resume()
{
	std::lock_guard lock{m_message_queue_mutex};
	std::string msg = m_message_queue.front();
	m_message_queue.pop_front();

	return msg;
}

} // namespace coactor::detail
