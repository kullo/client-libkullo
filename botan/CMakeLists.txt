#FIXME broken dependencies: cmake detects the dependencies wrongly.
# See
#     botan/CMakeFiles/botan.dir/depend.make
#     => *.cpp (bad performance), win32-msvc/*.h (incorrect)
# vs.
#     gcc -MM -std=c++11 botan/botan.cpp
#     => botan.cpp, $OS/* (correct)
kullo_add_simple_library(
    botan.cpp
)

kullo_get_target_name(target_name)
target_link_libraries(${target_name}
        PUBLIC
            # from other projects in workspace
                boost_filesystem
                boost_system
                z
)
if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_compile_options(${target_name}
        PUBLIC
            -Wno-pedantic
        PRIVATE
            -Wno-switch-default
    )
endif()
