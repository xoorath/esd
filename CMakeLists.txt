cmake_minimum_required (VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

if(MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MT>
        $<$<CONFIG:Debug>:/MTd>
        $<$<CONFIG:Release>:/MT>
        $<$<CONFIG:RelWithDebInfo>:/MTd>
        $<$<CONFIG:MinSizeRel>:/MT>
    )
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # This is specifically to pretty up packages made with github actions.
    # It removes the "/ConfigName" folders inside CMAKE_RUNTIME_OUTPUT_DIRECTORY
    if ((DEFINED GITHUB_CI) AND (${GITHUB_CI}))
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
      set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()
endif()

project (esd)

set(PROJ_PRIVATE_DIR     Source/ )

file(GLOB_RECURSE PROJ_SOURCE_FILES 
    "${PROJ_PRIVATE_DIR}/*.cpp"
    "${PROJ_PRIVATE_DIR}/*.h"
)

add_executable(${PROJECT_NAME} ${PROJ_SOURCE_FILES})

if(MSVC)
  set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Example")
  set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_COMMAND_ARGUMENTS "-v")
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()