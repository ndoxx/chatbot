cmake_minimum_required(VERSION 3.1.0)

SET (CMAKE_C_COMPILER             "/usr/bin/clang-7")
SET (CMAKE_C_FLAGS                "-Wall -std=c99")
SET (CMAKE_C_FLAGS_DEBUG          "-g")
SET (CMAKE_C_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
SET (CMAKE_C_FLAGS_RELEASE        "-O3 -DNDEBUG")
SET (CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")

SET (CMAKE_CXX_COMPILER             "/usr/bin/clang++-7")
SET (CMAKE_CXX_FLAGS                "-stdlib=libstdc++ -Wall -pedantic -Wno-unsupported-friend")
SET (CMAKE_CXX_FLAGS_DEBUG          "-g")
SET (CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
SET (CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
SET (CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -fno-omit-frame-pointer -fno-inline-functions -fno-optimize-sibling-calls")

SET (CMAKE_AR      "/usr/bin/llvm-ar-7")
SET (CMAKE_LINKER  "/usr/bin/llvm-link-7")
SET (CMAKE_NM      "/usr/bin/llvm-nm-7")
SET (CMAKE_OBJDUMP "/usr/bin/llvm-objdump-7")
SET (CMAKE_RANLIB  "/usr/bin/llvm-ranlib-7")

set(CMAKE_CXX_STANDARD 17)

# set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE MinSizeRel)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(chatbot_SRCS
    main.cpp
    stringselector.cpp
    song.cpp
    song_factory.cpp
    qa.cpp
)

# Tell CMake to create the chatbot executable
add_executable(chatbot ${chatbot_SRCS})
target_link_libraries(chatbot m)
