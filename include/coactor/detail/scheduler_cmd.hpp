#pragma once

#include <memory>
#include <string>
#include <variant>

#include <cstdint>

namespace coactor {
class Actor;
using ActorId = std::uint64_t;
} // namespace coactor

namespace coactor::detail {

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

} // namespace coactor::detail
