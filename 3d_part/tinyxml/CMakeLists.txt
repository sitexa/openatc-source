cmake_minimum_required(VERSION 3.0)

project(tinyxml)

add_compile_options(-fPIC)

set(TARGET_NAME tinyxml)

aux_source_directory(. TINYXML_SRCS)
add_library(${TARGET_NAME} ${TINYXML_SRCS})

SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES FOLDER "3d_part")
