cmake_minimum_required(VERSION 3.0)

project(OpenATCMainCtlManager)

# Set project name
PROJECT (OpenATCTest)

# Set Source Files
AUX_SOURCE_DIRECTORY(. SRCS)

add_definitions(-DCANOPEN_MASTER)


INCLUDE_DIRECTORIES(../Include)

add_subdirectory(comctl)
add_subdirectory(logicctl)

# Set -g flag for gdb debugger

if(WIN32)
else()
SET(CMAKE_CXX_FLAGS -g)
add_definitions(-D_LINUX_)
endif()

# Set Target Name
SET(TARGET_NAME OpenATCMainCtlManager)

# Set library type
SET(LIB_TYPE STATIC)

# Set linked librarys
SET(LINK_LIBS CommonLog OpenATCTZParam OpenATCGlobalStatus OpenATCFaultProc OpenATCCanBusProc OpenATCMainCtlSource intl)

if(DEFINED USE_EXTERNAL_ICONV)
    set(LINK_LIBS ${LINK_LIBS} iconv charset)
endif()

if(WIN32)
else()
set(LINK_LIBS ${LINK_LIBS} pthread rt)
endif()

# Set Executable and Library Output Path
SET(EXECUTABLE_OUTPUT_PATH OpenATCManager)

# Set Include Directorys
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Include)

# Setting Link Directorys
if(DEFINED VIRTUAL_DEVICE)
	LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../LibVir/Lib/)
elseif(WIN32)
	LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../LibWin/Lib/)
else()
	LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../Lib/)
endif()

if(DEFINED VIRTUAL_DEVICE)
	LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/build_msvc/external/lib)
elseif(WIN32)
	LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/build_msvc/external/lib)
else()
	LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/build_arm/external/lib)
endif()

# Build Executable
ADD_EXECUTABLE(${TARGET_NAME} ${SRCS})

# Link to create executables
TARGET_LINK_LIBRARIES(${TARGET_NAME} ${LINK_LIBS} comctl logicctl cjson RapidjsonHelper)

include_directories(
    ${PROJECT_SOURCE_DIR}/Include
    ${PROJECT_SOURCE_DIR}/Source
)