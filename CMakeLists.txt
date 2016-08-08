cmake_minimum_required(VERSION 2.8.12)  # version provided by Ubuntu 14.04

# for CMake < 3.0; see http://www.cmake.org/cmake/help/v3.0/policy/CMP0042.html
if(POLICY CMP0042)
	cmake_policy(SET CMP0042 NEW)
endif()

# These must come before the project directive, won't work otherwise

if(((NOT CMAKE_BUILD_TYPE) OR (CMAKE_BUILD_TYPE STREQUAL "Debug")) AND (NOT CMAKE_GENERATOR STREQUAL "Xcode"))
    set(install_prefix_suffix "-debug")
else()
    set(install_prefix_suffix "")
endif()
set(
    CMAKE_INSTALL_PREFIX
        "${CMAKE_CURRENT_BINARY_DIR}/../bin-libkullo${install_prefix_suffix}"
    CACHE PATH "destination for make install"
)

if(NOT ("${CMAKE_VERSION}" VERSION_LESS "3.1"))
    set(CMAKE_INSTALL_MESSAGE LAZY
        CACHE STRING "Show messages during install? Lazy means only changed.")
    set_property(CACHE CMAKE_INSTALL_MESSAGE
        PROPERTY STRINGS "ALWAYS" "LAZY" "NEVER")
endif()

project(libkullo)

set(_bin_botan "${CMAKE_CURRENT_BINARY_DIR}/../bin-botan${install_prefix_suffix}")
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${_bin_botan}/cmake")
include(CompilerSettings)
include(KulloCommon)

find_package(botan REQUIRED
    NO_MODULE
    PATHS "${_bin_botan}"
    NO_DEFAULT_PATH
)

### BEGIN feature checks

if(("${CMAKE_SYSTEM_NAME}" STREQUAL Linux) OR ("${CMAKE_SYSTEM_NAME}" STREQUAL Windows))
    set(HAS_BREAKPADWRAPPER TRUE)
else()
    set(HAS_BREAKPADWRAPPER FALSE)
endif()

if(IOS)
    set(TESTS_AS_STATIC_LIB TRUE)
else()
    set(TESTS_AS_STATIC_LIB FALSE)
endif()

### END feature checks

### BEGIN external libs

add_subdirectory("../boost" "external/boost")
add_subdirectory("../google-test" "external/google-test")
add_subdirectory("../smartsqlite/src" "external/smartsqlite/src")

### END external libs


### BEGIN internal libs

include_directories(.)

if(HAS_BREAKPADWRAPPER)
    add_subdirectory("breakpadwrapper")
endif()
if(ANDROID)
    add_subdirectory("jni")
endif()
add_subdirectory("jsoncpp")
add_subdirectory("kulloclient")
add_subdirectory("nowide")
add_subdirectory("tests")

### END internal libs

if(NOT TESTS_AS_STATIC_LIB)
    enable_testing()

    # smartsqlite
    add_subdirectory("../smartsqlite/test" "external/smartsqlite/test")
    add_test("smartsqlite" "external/smartsqlite/test/smartsqlite_tests")

    # libkullo
    add_test(
        "libkullo"
        "tests/libkullo_tests" --assets "${CMAKE_CURRENT_SOURCE_DIR}/tests/assets"
    )
endif()

install(DIRECTORY cmake/ DESTINATION cmake)
