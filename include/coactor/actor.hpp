#pragma once

#include "concurrent_queue.hpp"

#include <functional>

namespace coactor {

class Stage;

class Actor {
public:
	Actor(Stage& stage, std::function<Result<void>(Stage&, Actor&)> fun);
	~Actor();

	Result<void> run();
	Result<void> send(int i);

	Result<int> receive();
private:

	Stage& stage;
	ConcurrentQueue inbox;

	std::function<Result<void>(Stage&, Actor&)> fun;
};

} // namespace coactor
