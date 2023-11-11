add_library(coactor actor.cpp stage.cpp)
target_link_libraries(coactor PUBLIC concurrencpp::concurrencpp)
target_include_directories(coactor PUBLIC ${CMAKE_SOURCE_DIR}/include)
