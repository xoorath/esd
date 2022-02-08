cmake_minimum_required (VERSION 3.12)

if (MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.29.30129 AND CMAKE_VERSION VERSION_GREATER 3.20.3)
    # this change happened in CMake 3.20.4
    set(CMAKE_CXX_STANDARD 23) # /std:c++latest - unlocks the non stable cpp20 features. For new 16.11 versions
else ()
    set(CMAKE_CXX_STANDARD 20) # /std:c++latest for msvc and -std=c++20 for everyone else.
endif ()


project (esd)

set(PROJ_PRIVATE_DIR     Source/ )

file(GLOB_RECURSE PROJ_SOURCE_FILES 
    "${PROJ_PRIVATE_DIR}/*.cpp"
    "${PROJ_PRIVATE_DIR}/*.h"
)

add_executable(${PROJECT_NAME} ${PROJ_SOURCE_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Example")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})