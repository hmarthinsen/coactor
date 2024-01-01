add_library(coactor actor.cpp stage.cpp concurrent_queue.cpp message.cpp)
target_link_libraries(coactor PUBLIC concurrencpp::concurrencpp
                                     nlohmann_json::nlohmann_json)
target_include_directories(coactor PUBLIC ${CMAKE_SOURCE_DIR}/include)
