cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_CXX_STANDARD 17)

# set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE MinSizeRel)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(chatbot_SRCS
    main.cpp
    stringselector.cpp
    qa.cpp
)

# Tell CMake to create the chatbot executable
add_executable(chatbot ${chatbot_SRCS})
target_link_libraries(chatbot m)
