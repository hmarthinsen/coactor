add_library(coactor)
target_sources(
    coactor
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS ${PROJECT_SOURCE_DIR}/include
            FILES
                ${PROJECT_SOURCE_DIR}/include/coactor/actor.hpp
                ${PROJECT_SOURCE_DIR}/include/coactor/coactor.hpp
                ${PROJECT_SOURCE_DIR}/include/coactor/runtime.hpp
                ${PROJECT_SOURCE_DIR}/include/coactor/scheduler.hpp
                ${PROJECT_SOURCE_DIR}/include/coactor/detail/actor_coro.hpp
                ${PROJECT_SOURCE_DIR}/include/coactor/detail/utils.hpp
    PRIVATE
        actor.cpp
        runtime.cpp
        scheduler.cpp
        detail/actor_coro.cpp
        detail/utils.cpp
)
