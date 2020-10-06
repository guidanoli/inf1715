add_library(monga_utils STATIC
    monga_utils.c)

target_include_directories(monga_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})