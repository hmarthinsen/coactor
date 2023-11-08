add_library(coactor actor.cpp stage.cpp)
target_link_libraries(coactor PUBLIC concurrencpp::concurrencpp)
