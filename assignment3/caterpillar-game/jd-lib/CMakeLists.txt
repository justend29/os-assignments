cmake_minimum_required(VERSION 3.18)

project(
  jd-libs
  DESCRIPTION "C utility libraries"
  LANGUAGES C
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}) # default output static lib dir
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}) # default output dynamic lib dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTOR  ${PROJECT_BINARY_DIR}) # default executable dir
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Wsign-compare\
            -fdiagnostics-color=always")

# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    set(CMAKE_C_EXTENSIONS OFF) # Disable compiler specific extensions
    set_property(GLOBAL PROPERTY USE_FOLDERS ON) # Support folders in IDEs

    if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        include(CTest) # Enables testing and provides BUILD_TESTING option
    endif()

    # Docs only available if this is the main app
    find_package(Doxygen)
    if(Doxygen_FOUND)
        add_subdirectory(docs)
    else()
        message(STATUS "Doxygen not found, not building docs")
    endif()
endif()

# enable interprocedural linker optimization if available
include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_subdirectory(src)
