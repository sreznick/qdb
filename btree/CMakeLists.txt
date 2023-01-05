
add_library(btree btree.h btree.cc)

target_compile_features(btree PRIVATE cxx_std_20)

target_include_directories(btree PUBLIC
        "${PROJECT_SOURCE_DIR}"
        )


