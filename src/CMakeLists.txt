add_subdirectory(coactor)

add_executable(actortest actortest.cpp)
target_link_libraries(actortest PRIVATE coactor)

add_executable(reactive-co reactive-co.cpp)
target_link_libraries(reactive-co PRIVATE asio::asio fmt::fmt)

add_executable(ct conctest.cpp)
target_link_libraries(ct PRIVATE concurrencpp::concurrencpp)
