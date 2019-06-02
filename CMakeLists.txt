cmake_minimum_required(VERSION 3.0.0)
project(bstream VERSION 0.1.0)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "CMAKE_HOME_DIRECTORY = ${CMAKE_HOME_DIRECTORY}")

# set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/scripts")

	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize=address")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
#	else()
#		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O3")
	endif()

enable_testing()

set(Boost_USE_STATIC_LIBS   ON)

set(Boost_NO_SYSTEM_PATHS TRUE)
if (Boost_NO_SYSTEM_PATHS)
	set(BOOST_ROOT "${CMAKE_HOME_DIRECTORY}/../boost_1_68_0_install")
endif (Boost_NO_SYSTEM_PATHS)

find_package(Boost 1.68.0 REQUIRED system)

include(scripts/doutil.cmake)

include_directories( 
	include 
#	ext/boost
#	${CMAKE_HOME_DIRECTORY}/../boost_1_66_0
	${Boost_INCLUDE_DIRS}
	${CMAKE_HOME_DIRECTORY}/../filesystem/include
	${util_SOURCE_DIR}/include
	ext/doctest/include)

set(BSTREAM_SRCS
	src/bstream/error.cpp
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
# target_link_libraries(bstream_test bstream util ${Boost_LIBRARIES})
target_link_libraries(bstream_test bstream util ${Boost_LIBRARIES})

add_test(NAME bstream_test COMMAND bstream_test )
SET_TESTS_PROPERTIES(bstream_test
    PROPERTIES ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1")

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
