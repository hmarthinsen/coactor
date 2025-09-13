#include "coactor/detail/utils.hpp"

#include <atomic>
#include <chrono>
#include <format>
#include <iostream>

namespace coactor::detail {

std::uint64_t get_unique_id()
{
	static std::atomic<std::uint64_t> s_id_counter{0};

	return ++s_id_counter;
}

void log(
	[[maybe_unused]] std::string_view from,
	[[maybe_unused]] std::string_view msg
)
{
#ifndef NDEBUG
	static const auto time_start{std::chrono::system_clock::now()};
	std::cout << std::format(
		"{} [{}] - {}\n",
		std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now() - time_start
		),
		from,
		msg
	);
#endif
}

} // namespace coactor::detail
