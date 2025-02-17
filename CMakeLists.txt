cmake_minimum_required(VERSION 3.0)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x600)
else()
endif()

if(DEFINED XC_RULES_FILE) 
  INCLUDE(${CMAKE_SOURCE_DIR}/${XC_RULES_FILE})
endif()


project(OpenATC)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/build_msvc/external/lib/)
	    # 64 bits
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
		#LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/hwsdk/lib/x86)
	    # 32 bits
	endif()
endif()

##########################################
#   3d_part projects                                                                  #
##########################################
add_subdirectory(3d_part/tinyxml)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/3d_part/tinyxml)

add_subdirectory(3d_part/cjson)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/3d_part/cjson)

# rapidjson
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/3d_part/rapidjson)

# RapidjsonHelper
add_subdirectory(3d_part/RapidjsonHelper)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/3d_part/RapidjsonHelper)

#libintl-lite
add_subdirectory(3d_part/libintl-lite)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/3d_part/libintl-lite)


include_directories(${CMAKE_BINARY_DIR}/include)

##########################################
#   External depends!  Put 3rd_party depends here      #
##########################################
include_directories(${CMAKE_BINARY_DIR}/external/include)
link_directories(${CMAKE_BINARY_DIR}/external/lib)

if(DEFINED VIRTUAL_DEVICE)
	add_definitions(-DVIRTUAL_DEVICE)
endif()

if(WIN32)
    set(USE_EXTERNAL_ICONV 1)
endif()

# get rid of unused warning
set(ignoreMe "${VIRTUAL_DEVICE}${USE_EXTERNAL_ICONV}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/Include)

add_subdirectory(OpenATCManager)

find_package(Git)
add_custom_target(version
  ${CMAKE_COMMAND} -D SRC=${CMAKE_SOURCE_DIR}/Include/version.h.in
                   -D DST=${CMAKE_BINARY_DIR}/include/version.h
                   -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
                   -P ${CMAKE_SOURCE_DIR}/GenerateVersionHeader.cmake
  )
add_dependencies(OpenATCMainCtlManager version)

include_directories(
    ${PROJECT_SOURCE_DIR}/Include
    ${PROJECT_SOURCE_DIR}/Source
)