#pragma once

#include "coactor/result.hpp"

#include <concurrencpp/concurrencpp.h>

#include <memory>
#include <queue>
#include <vector>

#include <cstddef>

namespace coactor {

class Stage;

class ConcurrentQueue {
public:
	ConcurrentQueue(std::size_t max_queue_size = 1000)
		: m_max_queue_size{max_queue_size} {};

	Result<void> shutdown(Stage& stage);

	Result<void> push(
		std::shared_ptr<concurrencpp::executor> resume_executor,
		const std::vector<uint8_t>& data
	);
	Result<std::vector<uint8_t>>
	pop(std::shared_ptr<concurrencpp::executor> resume_executor);

private:
	concurrencpp::async_lock m_lock;
	concurrencpp::async_condition_variable m_cv;
	std::queue<std::vector<uint8_t>> m_queue;

	concurrencpp::async_condition_variable m_size_cv;
	std::size_t m_max_queue_size;

	bool m_shall_quit = false;
};

} // namespace coactor
