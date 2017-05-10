cmake_minimum_required(VERSION 3.0)
project(en-dump)
include_directories(src/windward)
add_executable(${PROJECT_NAME}
	src/main.cpp
	src/windward/DataFile.cpp src/windward/DataFile.h
)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
