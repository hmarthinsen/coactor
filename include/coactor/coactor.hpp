#pragma once

#include "coactor/detail/utils.hpp"
#include "scheduler.hpp"

#include <memory>
#include <utility>

namespace coactor {

template <typename Actor, typename... Args>
void run(Args... args)
{
	Scheduler scheduler;

	auto actor = std::make_unique<Actor>(args...);
	actor->set_name(detail::get_type_name<Actor>());
	scheduler.insert_actor(std::move(actor));

	scheduler.run();
}

} // namespace coactor
