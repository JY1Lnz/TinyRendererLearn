cmake_minimum_required(VERSION 3.10)
project(TinyRenderer)

file(GLOB SRC_FILE *.h *.cpp ../*.h ../*.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILE})