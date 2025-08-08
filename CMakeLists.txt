cmake_minimum_required(VERSION 3.10)

project(
  player
  VERSION 1.0
  LANGUAGES CXX)

add_executable(${PROJECT_NAME} main.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23
                                                 CXX_STANDARD_REQUIRED ON)
