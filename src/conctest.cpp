#include <concurrencpp/concurrencpp.h>

#include <iostream>
#include <queue>

using namespace concurrencpp;

class ConcurrentQueue {
private:
	async_lock m_lock;
	async_condition_variable m_cv;
	std::queue<int> m_queue;
	bool m_shall_quit = false;

public:
	ConcurrentQueue() = default;

	result<void> shutdown(std::shared_ptr<executor> resume_executor)
	{
		{
			auto guard = co_await m_lock.lock(resume_executor);
			m_shall_quit = true;
		}

		m_cv.notify_all();
	}

	lazy_result<void> push(std::shared_ptr<executor> resume_executor, int i)
	{
		{
			auto guard = co_await m_lock.lock(resume_executor);
			m_queue.push(i);
		}

		m_cv.notify_one();
	}

	lazy_result<int> pop(std::shared_ptr<executor> resume_executor)
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
};

result<void> producer_loop(
	executor_tag,
	std::shared_ptr<thread_pool_executor> tpe,
	ConcurrentQueue& queue,
	int range_start,
	int range_end
)
{
	for (; range_start < range_end; ++range_start) {
		co_await queue.push(tpe, range_start);
	}
}

result<void> consumer_loop(
	executor_tag,
	std::shared_ptr<thread_pool_executor> tpe,
	ConcurrentQueue& queue
)
{
	try {
		while (true) {
			std::cout << co_await queue.pop(tpe) << std::endl;
		}
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

int main()
{
	runtime runtime;
	const auto thread_pool_executor = runtime.thread_pool_executor();
	ConcurrentQueue queue;

	result<void> producers[4];
	result<void> consumers[4];

	for (int i = 0; i < 4; i++) {
		producers[i] = producer_loop(
			{}, thread_pool_executor, queue, i * 5, (i + 1) * 5
		);
	}

	for (int i = 0; i < 4; i++) {
		consumers[i] = consumer_loop({}, thread_pool_executor, queue);
	}

	for (int i = 0; i < 4; i++) {
		producers[i].get();
	}

	queue.shutdown(thread_pool_executor).get();

	for (int i = 0; i < 4; i++) {
		consumers[i].get();
	}

	return 0;
}