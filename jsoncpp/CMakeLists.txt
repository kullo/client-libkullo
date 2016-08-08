kullo_add_simple_library(
    jsoncpp-forwards.h
    jsoncpp.cpp
)

kullo_get_target_name(target_name)
if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_compile_options(${target_name}
        PUBLIC
            -Wno-format-nonliteral -Wno-switch-default
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL Clang)
    target_compile_options(${target_name}
        PRIVATE
            -Wno-covered-switch-default
            -Wno-deprecated
            -Wno-disabled-macro-expansion
            -Wno-float-equal
            -Wno-format-nonliteral
            -Wno-implicit-fallthrough
            -Wno-missing-noreturn
            -Wno-missing-variable-declarations
            -Wno-old-style-cast
            -Wno-sign-conversion
            -Wno-switch-enum
            -Wno-unused-macros
    )
endif()
