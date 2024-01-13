#pragma once

#include <concurrencpp/concurrencpp.h>

namespace coactor {

template <typename T> using Result = concurrencpp::result<T>;

} // namespace coactor
