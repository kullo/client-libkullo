if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(warn_flags "-Wall -Wextra -Wpedantic -Wcast-qual -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wswitch-default -Wuninitialized")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pthread ${warn_flags}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread ${warn_flags} -Woverloaded-virtual")

    # Hardening
    set(hardening_flags "-fstack-protector -pie -fPIE -D_FORTIFY_SOURCE=2 -Wl,-z,relro,-z,now")
    set(CMAKE_C_FLAGS_RELEASE          "${CMAKE_C_FLAGS_RELEASE}          ${hardening_flags}")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO}   ${hardening_flags}")
    set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}        ${hardening_flags}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${hardening_flags}")

# Workaround for CMake < 3.1: Variables such as 'MSVC' are expanded in 'if'
# statements even if they are quoted. We can't enable the policy CMP0054 because
# we need to support CMake 2.8. Therefore, we append a space on both sides,
# which disables automatic expansion.
elseif("${CMAKE_CXX_COMPILER_ID} " STREQUAL "MSVC ")
    # force gtest to use shared MSVC++ runtime
    set(gtest_force_shared_crt ON)
endif()

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

	if(NOT IOS)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.9")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.9")
	endif()
endif()
