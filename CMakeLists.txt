cmake_minimum_required(VERSION 3.14)
project(ShamirSecret)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/scripts")
include(DirectorySeacher)

include_directories(ShamirSecret ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(ShamirSecret ${CMAKE_CURRENT_SOURCE_DIR}/BigInt)
include_directories(ShamirSecret ${CMAKE_CURRENT_SOURCE_DIR}/Utilits)

add_dir_to_executable(ShamirSecret ${CMAKE_CURRENT_SOURCE_DIR})
dir_to_target_sources(ShamirSecret ${CMAKE_CURRENT_SOURCE_DIR}/BigInt)
dir_to_target_sources(ShamirSecret ${CMAKE_CURRENT_SOURCE_DIR}/Utilits)
