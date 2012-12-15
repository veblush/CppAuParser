include_directories(../include/)

add_executable(tutorial1 tutorial1.cpp)
target_link_libraries(tutorial1 cppauparser)

add_executable(tutorial2 tutorial2.cpp)
target_link_libraries(tutorial2 cppauparser)

add_executable(tutorial3 tutorial3.cpp)
target_link_libraries(tutorial3 cppauparser)

add_executable(tutorial4 tutorial4.cpp)
target_link_libraries(tutorial4 cppauparser)

add_executable(tutorial5 tutorial5.cpp)
target_link_libraries(tutorial5 cppauparser)

add_executable(sample-list sample-list.cpp)
target_link_libraries(sample-list cppauparser)

add_executable(sample-json sample-json.cpp)
target_link_libraries(sample-json cppauparser)

add_executable(sample-benchmark sample-benchmark.cpp)
target_link_libraries(sample-benchmark cppauparser)
