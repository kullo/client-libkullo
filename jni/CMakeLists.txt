set(target_name kullo)

include(api/source_files.cmake)
include(http/source_files.cmake)
include(support-lib/source_files.cmake)
add_library(${target_name} SHARED
	${api_jni_source_files}
	${http_jni_source_files}
	${jni_supportlib_source_files}
	exceptions.cpp
)

target_include_directories(${target_name} PRIVATE support-lib)
target_link_libraries(${target_name} kulloclient)

install(TARGETS ${target_name} LIBRARY DESTINATION lib)
