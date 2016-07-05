include_directories("../../build-google-breakpad/src")

if("${CMAKE_SYSTEM_NAME}" STREQUAL Linux)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-pedantic")
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
