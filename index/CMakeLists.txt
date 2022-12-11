
add_library(index index.cc btree.cc)

target_compile_features(index PRIVATE cxx_std_20)

target_include_directories(index PUBLIC
                          "${PROJECT_SOURCE_DIR}"
                          )

