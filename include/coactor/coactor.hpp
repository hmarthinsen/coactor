#pragma once

#include "coactor/detail/utils.hpp"
#include "scheduler.hpp"

#include <format>
#include <memory>
#include <utility>

namespace coactor {

template <typename ActorT, typename... Args>
void run(Args... args)
{
	Scheduler scheduler;

	const std::string_view actor_type_name = detail::get_type_name<ActorT>();

	auto actor = std::make_unique<ActorT>(args...);
	Address address = detail::get_unique_id();
	actor->set_address(address);
	actor->set_name(actor_type_name);

	detail::log(
		"System",
		std::format("{} {}", detail::bold("Spawning"), *actor)
	);

	scheduler.insert_actor(std::move(actor));
	scheduler.run();
}

} // namespace coactor
