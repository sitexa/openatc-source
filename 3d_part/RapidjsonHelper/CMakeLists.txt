cmake_minimum_required(VERSION 3.0)

project(RapidjsonHelper)

set(TARGET_NAME RapidjsonHelper)

add_compile_options(-fPIC)

add_library(${TARGET_NAME} STATIC RapidjsonHelper.h  RapidjsonHelper.cpp)

SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES FOLDER "3d_part")
