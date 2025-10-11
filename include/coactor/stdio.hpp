#pragma once

#include "coactor/actor.hpp"

namespace coactor {

// There are two possible commands:
// "in <reply_addr> <prompt>"
// "out <msg>"
class StdIo : public Actor {
private:
	Coroutine act() override;
};

} // namespace coactor
