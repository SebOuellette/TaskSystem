cmake_minimum_required(VERSION 3.7)

project(tasksystem)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(Threads)
find_package(CURL REQUIRED)

# Include directories for MySQL Connector/C++
include_directories(${Boost_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})

# Add your source files
add_executable(tasksystem main.cpp)

# Link against Boost, Threads, MySQL Connector/C++, and libcurl
target_link_libraries(tasksystem ${Boost_LIBRARIES} Threads::Threads ${CURL_LIBRARIES} /usr/lib/x86_64-linux-gnu/libsqlite3.so.0)