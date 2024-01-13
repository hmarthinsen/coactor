add_subdirectory(coactor)
add_subdirectory(experiments)

add_executable(actortest actortest.cpp)
target_link_libraries(actortest PRIVATE coactor)

add_executable(reactive-co reactive-co.cpp)
target_link_libraries(reactive-co PRIVATE asio::asio)

add_executable(ct conctest.cpp)
target_link_libraries(ct PRIVATE concurrencpp::concurrencpp)

add_executable(cotest cotest.cpp)
