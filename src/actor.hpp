#pragma once

#include "concurrent_queue.hpp"

#include <functional>

namespace coactor {

class Stage;

class Actor {
public:
	Actor(Stage& stage, std::function<result<void>(Stage&, Actor&)> fun);
	~Actor();

	result<int> receive();
	result<void> send(int i);

	result<void> run();

	Stage& stage;
	ConcurrentQueue inbox;

	std::function<result<void>(Stage&, Actor&)> fun;
};

} // namespace coactor
