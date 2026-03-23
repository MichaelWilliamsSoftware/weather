cmake_minimum_required(VERSION 3.10)
project(weather)

set(CMAKE_C_STANDARD 99)

find_package(CURL REQUIRED)
find_package(cJSON REQUIRED)
find_package(SQLite3 REQUIRED)

add_executable(weather
    src/main.c
)

target_link_libraries(weather
    CURL::libcurl
    cjson
    SQLite::SQLite3
)