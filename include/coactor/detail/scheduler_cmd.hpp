#pragma once

#include <memory>
#include <string>
#include <variant>

#include <cstdint>

namespace coactor {
class Actor;
using Address = std::uint64_t;
} // namespace coactor

namespace coactor::detail {

struct SpawnCommand {
	std::unique_ptr<Actor> actor;
};

struct SendCommand {
	Address sender;
	Address receiver;
	std::string msg;
};

struct ReceiveCommand { };

using SchedulerCommand
	= std::variant<std::monostate, SpawnCommand, SendCommand, ReceiveCommand>;

} // namespace coactor::detail
