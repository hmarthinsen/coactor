#pragma once

#include <concurrencpp/concurrencpp.h>

#include <functional>
#include <memory>

namespace coactor {

template <typename T> using Result = concurrencpp::result<T>;

class Actor;

class Stage {
public:
	std::shared_ptr<concurrencpp::thread_pool_executor> get_executor();

	Actor spawn_actor(std::function<Result<void>(Stage&, Actor&)> fun);

private:
	concurrencpp::runtime m_runtime;
};

} // namespace coactor