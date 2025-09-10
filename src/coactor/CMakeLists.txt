add_library(coactor)
target_sources(
    coactor
    PUBLIC
        FILE_SET HEADERS
            BASE_DIRS ${PROJECT_SOURCE_DIR}/include
            FILES
                ${PROJECT_SOURCE_DIR}/include/coactor/actor.hpp
                ${PROJECT_SOURCE_DIR}/include/coactor/coactor.hpp
                ${PROJECT_SOURCE_DIR}/include/coactor/scheduler.hpp
    PRIVATE actor.cpp scheduler.cpp
)
