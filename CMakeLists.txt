cmake_minimum_required(VERSION 3.16)

project(qdb VERSION 1.0)

add_subdirectory(common)
add_subdirectory(storage)
add_subdirectory(index)
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
target_link_libraries(qdb PUBLIC common storage pagecache index)
target_compile_features(qdb PRIVATE cxx_std_20)

