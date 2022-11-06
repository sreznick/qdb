
add_library(storage storage.cc)

target_compile_features(storage PRIVATE cxx_std_20)

target_include_directories(storage PUBLIC
                          "${PROJECT_SOURCE_DIR}"
                          )


