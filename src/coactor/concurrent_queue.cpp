#include "coactor/concurrent_queue.hpp"

#include "coactor/stage.hpp"

namespace coactor {
Result<void> ConcurrentQueue::shutdown(Stage& stage)
{
	const auto resume_executor = stage.get_executor();
	{
		auto guard = co_await m_lock.lock(resume_executor);
		m_shall_quit = true;
	}

	m_cv.notify_all();
}

Result<void> ConcurrentQueue::push(
	std::shared_ptr<concurrencpp::executor> resume_executor, int i
)
{
	{
		auto guard = co_await m_lock.lock(resume_executor);
		m_queue.push(i);
	}

	m_cv.notify_one();
}

Result<int>
ConcurrentQueue::pop(std::shared_ptr<concurrencpp::executor> resume_executor)
{
	auto guard = co_await m_lock.lock(resume_executor);
	co_await m_cv.await(resume_executor, guard, [this] {
		return m_shall_quit || !m_queue.empty();
	});

	if (!m_queue.empty()) {
		auto result = m_queue.front();
		m_queue.pop();

		co_return result;
	}

	assert(m_shall_quit);
	throw std::runtime_error("queue has been shut down.");
}
} // namespace coactor
