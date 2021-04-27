cmake_minimum_required(VERSION 3.19)
project(EDJournal_server)

set(CMAKE_CXX_STANDARD 20)

find_package(websocketpp CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

add_executable(EDJournal_server main.cpp)

target_include_directories(EDJournal_server PRIVATE ${RAPIDJSON_INCLUDE_DIRS})

target_link_libraries(EDJournal_server PRIVATE websocketpp::websocketpp)