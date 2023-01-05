
add_library(qengine qengine.cc filter.cc filter.h access_method.cc access_method.h)

target_compile_features(qengine PRIVATE cxx_std_20)

target_include_directories(qengine PUBLIC
                          "${PROJECT_SOURCE_DIR}"
                          )

