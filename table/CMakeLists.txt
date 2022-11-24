
add_library(table table.cc types.cc)

target_compile_features(table PRIVATE cxx_std_20)

target_include_directories(table PUBLIC
                          "${PROJECT_SOURCE_DIR}"
                          )

