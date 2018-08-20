if("${CMAKE_SYSTEM_NAME}" STREQUAL Linux)
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        include_directories("../../build-linux-x86_64-debug/build-breakpad/src")
    else()
        include_directories("../../build-linux-x86_64-release/build-breakpad/src")
    endif()
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL Windows)
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        include_directories("../../build-windows-x86_32-debug/build-breakpad/src")
    else()
        include_directories("../../build-windows-x86_32-release/build-breakpad/src")
    endif()
else()
    message(FATAL_ERROR "os detection failed")
endif()

if("${CMAKE_SYSTEM_NAME}" STREQUAL Linux)
    set(platform_sources breakpad_linux.cpp)

elseif("${CMAKE_SYSTEM_NAME}" STREQUAL Windows)
    set(platform_sources breakpad_windows.cpp)
endif()


kullo_add_simple_library(
    breakpad_all.cpp
    ${platform_sources}
)
kullo_get_target_name(target_name)
target_link_libraries(${target_name} PUBLIC boost_filesystem)
