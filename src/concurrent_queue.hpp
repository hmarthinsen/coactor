#pragma once

#include "stage.hpp"

#include <concurrencpp/concurrencpp.h>

#include <memory>
#include <queue>
#include <stdexcept>

namespace coactor {

class ConcurrentQueue {
public:
	ConcurrentQueue() = default;

	concurrencpp::result<void> shutdown(Stage& stage)
	{
		const auto resume_executor = stage.get_executor();
		{
			auto guard = co_await m_lock.lock(resume_executor);
			m_shall_quit = true;
		}

		m_cv.notify_all();
	}

	concurrencpp::lazy_result<void>
	push(std::shared_ptr<concurrencpp::executor> resume_executor, int i)
	{
		{
			auto guard = co_await m_lock.lock(resume_executor);
			m_queue.push(i);
		}

		m_cv.notify_one();
	}

	concurrencpp::lazy_result<int>
	pop(std::shared_ptr<concurrencpp::executor> resume_executor)
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

private:
	concurrencpp::async_lock m_lock;
	concurrencpp::async_condition_variable m_cv;
	std::queue<int> m_queue;
	bool m_shall_quit = false;
};

} // namespace coactor
