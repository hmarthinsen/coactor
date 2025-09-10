#pragma once

#include "scheduler6.hpp"

namespace coactor {

template <typename Actor, typename... Args>
void run(Args... args)
{
	Scheduler scheduler;
	scheduler.insert_actor(std::make_unique<Actor>(args...));

	scheduler.run();
}

} // namespace coactor
