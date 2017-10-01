kullo_add_simple_library(
    argon2.h
    argon2.c
    blake2/blake2b.c
    blake2/blake2.h
    blake2/blake2-impl.h
    blake2/blamka-round-ref.h
    core.c
    core.h
    encoding.c
    encoding.h
    ref.c
    thread.c
    thread.h
)

kullo_get_target_name(target_name)
if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_compile_options(${target_name}
    )
elseif(CMAKE_C_COMPILER_ID STREQUAL Clang)
    target_compile_options(${target_name}
        PRIVATE
            -Wno-cast-qual
            -Wno-missing-noreturn
            -Wno-reserved-id-macro
            -Wno-sign-conversion
            -Wno-undef
            -Wno-unused-macros
    )
endif()
