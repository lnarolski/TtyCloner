cmake_minimum_required(VERSION 3.13)

# set the project name
project(TtyCloner)

# add the executable
add_executable(TtyCloner main.cpp)

TARGET_LINK_LIBRARIES(TtyCloner util)