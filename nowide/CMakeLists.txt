kullo_add_simple_library(
    iostream.cpp
)

kullo_get_target_name(target_name)
if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_compile_options(${target_name} PUBLIC -Wno-switch-default)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    target_compile_options(${target_name} PRIVATE -w)
endif()
