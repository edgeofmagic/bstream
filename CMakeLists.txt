cmake_minimum_required(VERSION 3.0.0)
project(bstream VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "CMAKE_HOME_DIRECTORY = ${CMAKE_HOME_DIRECTORY}")

# set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/scripts")

	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
#	else()
#		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3")
	endif()

enable_testing()

include(scripts/doutil.cmake)

# add_subdirectory( ext/util )

# INCLUDE_DIRECTORIES(/Users/dcurtis/botan-install/include/botan-2)
# LINK_DIRECTORIES(/Users/dcurtis/botan-install/lib)

include_directories( 
	include 
	ext/boost 
#	ext/util/include
	${util_SOURCE_DIR}/include
	ext/doctest/include)

set(BSTREAM_SRCS
	include/bstream.h
	include/bstream/base_classes.h
	include/bstream/context.h
	include/bstream/error.h
	include/bstream/error_category_context.h
	include/bstream/fwd_decls.h
	include/bstream/ibstream.h
	include/bstream/ibstream_traits.h
	include/bstream/ifbstream.h
	include/bstream/imbstream.h
	include/bstream/macros.h
	include/bstream/msgpack.h
	include/bstream/obstream.h
	include/bstream/obstream_traits.h
	include/bstream/ofbstream.h
	include/bstream/ombstream.h
	include/bstream/stdlib.h
	include/bstream/typecode.h
	include/bstream/types.h
	include/bstream/source.h
	include/bstream/sink.h
#	include/bstream/seq_sink.h
	include/bstream/file/source.h
	include/bstream/file/sink.h
	include/bstream/buffer/source.h
	include/bstream/buffer/sink.h
	include/bstream/bufseq/sink.h
	include/bstream/stdlib/chrono.h
	include/bstream/stdlib/deque.h
	include/bstream/stdlib/forward_list.h
	include/bstream/stdlib/list.h
	include/bstream/stdlib/map.h
	include/bstream/stdlib/pair.h
	include/bstream/stdlib/set.h
	include/bstream/stdlib/tuple.h
	include/bstream/stdlib/unordered_map.h
	include/bstream/stdlib/unordered_set.h
	include/bstream/stdlib/vector.h
	include/bstream/memory.h

	src/bstream/error.cpp
	src/bstream/error_category_context.cpp
	src/bstream/ibstream.cpp
	src/bstream/obstream.cpp
	src/bstream/source.cpp
	src/bstream/sink.cpp
#	src/bstream/seq_sink.cpp
	src/bstream/file_source.cpp
	src/bstream/file_sink.cpp
	src/bstream/buffer_sink.cpp
	src/bstream/bufseq_sink.cpp)

set(BSTREAM_TEST_SRCS
	test/bstream/abstract_source.cpp
	test/bstream/file.cpp
	test/bstream/memory.cpp
	test/bstream/bufseq.cpp
	test/bstream/fbstream.cpp
	test/bstream/test0.cpp
	test/bstream/test1.cpp
#	test/bstream/test2.cpp
	test/bstream/test3.cpp
	test/bstream/test4.cpp
	test/test_main.cpp)

add_library(bstream ${BSTREAM_SRCS})

add_executable(bstream_test ${BSTREAM_TEST_SRCS})
target_link_libraries(bstream_test bstream util)

#add_test(NAME test_bstream COMMAND bstream_test --test-case=*bstream*)
#add_test(NAME test_async COMMAND bstream_test --test-case=*async*)

add_test(NAME bstream_test COMMAND bstream_test )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(bstream_doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
