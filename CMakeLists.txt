cmake_minimum_required(VERSION 3.16)

project(qdb VERSION 1.0)

add_subdirectory(common)
add_subdirectory(storage)
add_subdirectory(pagecache)


set(SRC_DIR "src")
set(PARSER_DIR "${CMAKE_CURRENT_BINARY_DIR}")

find_package(FLEX 2.6 REQUIRED)
find_package(BISON 3.0 REQUIRED)

set(LEXER_OUT "${PARSER_DIR}/lexer.cc")
set(PARSER_OUT "${PARSER_DIR}/parser.cc")

flex_target(LEXER "${SRC_DIR}/lexer.l" "${LEXER_OUT}" DEFINES_FILE "${PARSER_DIR}/lexer.h")
bison_target(PARSER "${SRC_DIR}/parser.y" "${PARSER_OUT}" DEFINES_FILE "${PARSER_DIR}/parser.h")
add_flex_bison_dependency(LEXER PARSER)

add_executable(qdb "${SRC_DIR}/main.cc" "${LEXER_OUT}" "${PARSER_OUT}")
target_include_directories(qdb PRIVATE "${PARSER_DIR}" "${SRC_DIR}" "${PROJECT_SOURCE_DIR}/common")
target_link_libraries(qdb PUBLIC common storage pagecache)
target_compile_features(qdb PRIVATE cxx_std_20)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/af29db7ec28d6df1c7f0f745186884091e602e07.zip
)

FetchContent_MakeAvailable(googletest)
include(GoogleTest)

enable_testing()

add_executable(
        test1
        pagecache
        storage
        tests/test_1.cc
)
target_link_libraries(
        test1
        storage
        pagecache
        gtest_main
)
target_compile_features(test1 PUBLIC cxx_std_20)

target_include_directories(test1 PRIVATE storage pagecache common)
gtest_discover_tests(test1)