#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <variant>

namespace coactor {

class Actor;

// TODO: Use an opaque ID, not pointer.
using ActorId = void*;

namespace detail {

struct SpawnCommand {
	std::unique_ptr<Actor> actor;
};

struct SendCommand {
	ActorId receiver;
	std::string msg;
};

struct ReceiveCommand { };

using SchedulerCommand
	= std::variant<std::monostate, SpawnCommand, SendCommand, ReceiveCommand>;

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

} // namespace detail

} // namespace coactor
