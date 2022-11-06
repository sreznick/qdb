
add_library(pagecache pagecache.cc)

target_compile_features(pagecache PRIVATE cxx_std_20)

target_include_directories(pagecache PUBLIC
                          "${PROJECT_SOURCE_DIR}"
                          )

