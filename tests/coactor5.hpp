#pragma once

#include "scheduler5.hpp"

namespace coactor {

template <typename Actor, typename... Args>
void run(Args... args)
{
	coactor::Scheduler scheduler;
	scheduler.insert(std::make_unique<Actor>(args...));

	scheduler.run();
}

} // namespace coactor
