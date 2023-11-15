#pragma once

#include <concurrencpp/concurrencpp.h>

#include <memory>
#include <queue>
#include <stdexcept>

namespace coactor {

template <typename T> using Result = concurrencpp::result<T>;

class Stage;

class ConcurrentQueue {
public:
	ConcurrentQueue() = default;

	Result<void> shutdown(Stage& stage);

	Result<void>
	push(std::shared_ptr<concurrencpp::executor> resume_executor, int i);
	Result<int> pop(std::shared_ptr<concurrencpp::executor> resume_executor);

private:
	concurrencpp::async_lock m_lock;
	concurrencpp::async_condition_variable m_cv;
	std::queue<int> m_queue;
	bool m_shall_quit = false;
};

} // namespace coactor
