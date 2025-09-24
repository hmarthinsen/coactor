#pragma once

#include "coactor/actor.hpp" // IWYU pragma: export
#include "coactor/runtime.hpp"

namespace coactor {

// Convenience function for starting an actor in the runtime with the
// default number of threads. Blocks until done.
template <typename ActorT, typename... Args>
int run(Args... args)
{
	coactor::Runtime runtime;
	return runtime.run<ActorT>(args...);
}

} // namespace coactor
