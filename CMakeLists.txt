cmake_minimum_required(VERSION 3.29)
project(sqlite)

set(CMAKE_CXX_STANDARD 20)
find_package(Boost REQUIRED)
find_package(date REQUIRED)
find_package(fmt REQUIRED)
find_package(range-v3 REQUIRED)

add_library(sqlite STATIC
        value.h
        sqlite.cpp sqlite.h
        types.h
        logger.h
        shared.h
        field.h
        query.h
        result.h
        row.h
        stmt.cpp stmt.h
        field.cc
        row.cc
        value.cc
        result.cc
        query.cc
        gzip.h
)

target_link_libraries(sqlite PRIVATE
        sqlite3
        fmt::fmt
        date::date date::date-tz
        range-v3::meta range-v3::concepts range-v3::range-v3
        libboost_iostreams.a
        z
)