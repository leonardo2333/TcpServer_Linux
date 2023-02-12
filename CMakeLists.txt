project(Linux_Test)

SET(CMAKE_BUILD_TYPE "Debug")
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/bin)





add_executable(server ./src/server.cpp)
add_library(tcp_server SHARED ./include/tcp_server.hpp ./include/tcp_server.cpp)
target_link_libraries(server tcp_server)

add_executable(client ./src/client.cpp)
add_library(tcp_client SHARED ./include/tcp_client.hpp ./include/tcp_client.cpp)
target_link_libraries(client tcp_client)

#链接pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads)
target_link_libraries(server ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT})