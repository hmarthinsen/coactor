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

std::string colorize(std::string_view str, std::uint64_t i)
{
	// Red is reserved for errors.
	int color_code = i % 5 + 32;
	return std::format("\033[{}m{}\033[0m", color_code, str);
}

std::string red(std::string_view str)
{
	return std::format("\033[31m{}\033[0m", str);
}
std::string green(std::string_view str)
{
	return std::format("\033[32m{}\033[0m", str);
}
std::string yellow(std::string_view str)
{
	return std::format("\033[33m{}\033[0m", str);
}
std::string blue(std::string_view str)
{
	return std::format("\033[34m{}\033[0m", str);
}
std::string magenta(std::string_view str)
{
	return std::format("\033[35m{}\033[0m", str);
}
std::string cyan(std::string_view str)
{
	return std::format("\033[36m{}\033[0m", str);
}
std::string bold(std::string_view str)
{
	return std::format("\033[1m{}\033[0m", str);
}
std::string italic(std::string_view str)
{
	return std::format("\033[3m{}\033[0m", str);
}

} // namespace coactor::detail
