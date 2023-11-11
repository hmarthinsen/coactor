#include "coactor/actor.hpp"

#include "coactor/stage.hpp"

namespace coactor {

Actor::Actor(Stage& stage, std::function<Result<void>(Stage&, Actor&)> fun)
	: stage{stage}, fun{fun}
{
}

Actor::~Actor() { inbox.shutdown(stage).get(); }

Result<int> Actor::receive()
{
	const auto tpe = stage.get_executor();
	return inbox.pop(tpe).run();
}

Result<void> Actor::send(int i)
{
	const auto tpe = stage.get_executor();
	return inbox.push(tpe, i).run();
}

Result<void> Actor::run() { return fun(stage, *this); }

} // namespace coactor
