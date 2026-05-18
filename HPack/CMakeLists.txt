add_library(hpack STATIC)
target_include_directories(hpack PUBLIC "HPack.h" "${CMAKE_CURRENT_SOURCE_DIR}/Vendor/")
target_sources(hpack PRIVATE "HPack.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Vendor/stb_impl.cpp")
