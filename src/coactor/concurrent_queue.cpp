#include "coactor/concurrent_queue.hpp"

#include "coactor/stage.hpp"

#include <stdexcept>

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
	std::shared_ptr<concurrencpp::executor> resume_executor,
	const std::vector<uint8_t>& data
)
{
	{
		auto guard = co_await m_lock.lock(resume_executor);

		co_await m_size_cv.await(resume_executor, guard, [this] {
			return m_queue.size() < m_max_queue_size;
		});

		m_queue.push(data);
	}

	m_cv.notify_one();
}

Result<std::vector<uint8_t>>
ConcurrentQueue::pop(std::shared_ptr<concurrencpp::executor> resume_executor)
{
	auto guard = co_await m_lock.lock(resume_executor);
	co_await m_cv.await(resume_executor, guard, [this] {
		return m_shall_quit || !m_queue.empty();
	});

	if (!m_queue.empty()) {
		auto data = m_queue.front();
		m_queue.pop();

		m_size_cv.notify_one();

		co_return data;
	}

	assert(m_shall_quit);
	throw std::runtime_error("queue has been shut down.");
}
} // namespace coactor
