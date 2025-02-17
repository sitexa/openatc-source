cmake_minimum_required(VERSION 3.0)

project(cjson)

add_compile_options(-fPIC)

set(TARGET_NAME cjson)

add_library(${TARGET_NAME} STATIC cJSON.h  cJSON.cpp)

SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES FOLDER "3d_part")
