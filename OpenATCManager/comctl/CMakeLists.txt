cmake_minimum_required(VERSION 3.0)


aux_source_directory(. COMCTL_SRCS)
add_library(comctl ${COMCTL_SRCS})
