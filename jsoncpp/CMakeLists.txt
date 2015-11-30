kullo_add_simple_library(
    jsoncpp-forwards.h
    jsoncpp.cpp
)

kullo_get_target_name(target_name)
if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	target_compile_options(${target_name} PUBLIC -Wno-format-nonliteral -Wno-switch-default)
endif()
