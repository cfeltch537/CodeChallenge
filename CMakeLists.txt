cmake_minimum_required (VERSION 3.5)

project (CodeChallenge)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/")
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++14 -pthread")
set (source_dir "${PROJECT_SOURCE_DIR}/src/")
set (include_dir "${PROJECT_SOURCE_DIR}/include/")


file (GLOB server_source_files "${source_dir}/server/*.cpp")
file (GLOB client_source_files "${source_dir}/client/*.cpp")
file (GLOB shared_header_files "${include_dir}/*.hpp")


#set(HEADER_FILES ${include_dir})
#add_library(connection_lib ${HEADER_FILES})

find_package( Boost REQUIRED COMPONENTS serialization system filesystem thread)
include_directories(${BOOST_INCLUDE_DIRS})

add_executable (server ${server_source_files} ${shared_header_files})
target_link_libraries(server ${Boost_LIBRARIES})
#target_include_directories(server PRIVATE ${shared_header_files})


add_executable (client ${client_source_files} ${shared_header_files})
target_link_libraries(client ${Boost_LIBRARIES})
#target_include_directories(client PRIVATE ${shared_header_files})

install (TARGETS server client LIBRARY DESTINATION lib/ RUNTIME DESTINATION bin/)

