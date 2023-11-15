add_library(coactor actor.cpp stage.cpp concurrent_queue.cpp)
target_link_libraries(coactor PUBLIC concurrencpp::concurrencpp)
target_include_directories(coactor PUBLIC ${CMAKE_SOURCE_DIR}/include)
