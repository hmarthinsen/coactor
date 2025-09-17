#pragma once

#include <string>
#include <string_view>

#include <cstdint>

namespace coactor::detail {

template <typename T>
constexpr auto get_type_name() -> std::string_view
{
#if defined(__clang__)
	constexpr auto prefix = std::string_view{"[T = "};
	constexpr auto suffix = "]";
	constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(__GNUC__)
	constexpr auto prefix = std::string_view{"with T = "};
	constexpr auto suffix = "; ";
	constexpr auto function = std::string_view{__PRETTY_FUNCTION__};
#elif defined(_MSC_VER)
	constexpr auto prefix = std::string_view{"get_type_name<"};
	constexpr auto suffix = ">(void)";
	constexpr auto function = std::string_view{__FUNCSIG__};
#else
#	error Unsupported compiler
#endif

	const auto start = function.find(prefix) + prefix.size();
	const auto end = function.find(suffix);
	const auto size = end - start;

	return function.substr(start, size);
}

std::uint64_t get_unique_id();

void log(std::string_view from, std::string_view msg);

std::string colorize(std::string_view str, std::uint64_t i);
std::string red(std::string_view str);
std::string green(std::string_view str);
std::string yellow(std::string_view str);
std::string blue(std::string_view str);
std::string magenta(std::string_view str);
std::string cyan(std::string_view str);
std::string bold(std::string_view str);
std::string italic(std::string_view str);

} // namespace coactor::detail
