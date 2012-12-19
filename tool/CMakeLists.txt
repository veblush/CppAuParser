include_directories(../include/)

add_executable(auparser-tool auparser-tool.cpp)
target_link_libraries(auparser-tool cppauparser)

install (TARGETS auparser-tool DESTINATION bin)
