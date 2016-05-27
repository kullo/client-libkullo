# Store the canonical target name in the variable with the given name.
#
# Usage: kullo_get_target_name(target_name)
function(kullo_get_target_name varname)
    # take the last component of the source dir, so "foo/bar/baz" becomes "baz"
    get_filename_component(result "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

    # "return" the value
    set(${varname} "${result}" PARENT_SCOPE)
endfunction()


# Install the given library target.
#
# Usage: kullo_install_lib(mylib)
function(kullo_install_lib target_name)
    install(TARGETS ${target_name}
        EXPORT ${CMAKE_PROJECT_NAME}
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include
    )
    install(EXPORT ${CMAKE_PROJECT_NAME} DESTINATION cmake/${CMAKE_PROJECT_NAME})
endfunction()


# Take a base directory and a list of files and reproduce the structure of the
# files inside the base directory.
#
# Usage: kullo_install_files(include header1.h header2.h ...)
function(kullo_install_files basedir)
    # ARGV = basedir file1 file2 ...
    set(files ${ARGV})
    list(REMOVE_AT files 0)

    foreach(file ${files})
        get_filename_component(dir "${file}" DIRECTORY)
        install(FILES ${file} DESTINATION ${basedir}/${dir})
    endforeach()
endfunction()


# Add and install a library with headers.
#
# Usage: kullo_add_library(mylib "foo.h;bar.h" "foo.cpp;bar.cpp")
function(kullo_add_library target_name headers sources)
    add_library(${target_name} ${headers} ${sources})
    kullo_install_lib(${target_name})
    kullo_install_files("include/${target_name}" ${headers})
endfunction()


# Add a static library.
#
# Usage: kullo_add_simple_library(file1.cpp file2.cpp ...)
function(kullo_add_simple_library)

    kullo_get_target_name(target_name)

    # get all headers from source dir
    file(GLOB_RECURSE headers
        RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
        "*.h" "*.hpp"
    )

    set(sources ${ARGV})

    kullo_add_library(${target_name} "${headers}" "${sources}")
endfunction()
