cmake_minimum_required(VERSION 3.10)

project(Transport_catalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto map_renderer.proto transport_router.proto)

set(TRANSPORT_CATALOGUE_FILES ranges.h domain.cpp request_handler.cpp domain.h request_handler.h geo.cpp router.h geo.h serialization.cpp graph.h serialization.h json.cpp svg.cpp json.h svg.h json_builder.cpp transport_catalogue.cpp json_builder.h transport_catalogue.h json_reader.cpp json_reader.h transport_router.cpp main.cpp transport_router.h map_renderer.cpp map_renderer.h)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORT_CATALOGUE_FILES})

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)