
add_library(common common.cc)

target_compile_features(common PRIVATE cxx_std_20)

target_include_directories(common PUBLIC
                          "${PROJECT_SOURCE_DIR}"
                          )


