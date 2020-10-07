add_subdirectory(utils)
add_subdirectory(ast)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(BISON_OUTPUT_C ${CMAKE_CURRENT_BINARY_DIR}/monga.y.c)
set(BISON_OUTPUT_H ${CMAKE_CURRENT_BINARY_DIR}/monga.y.h)
if (BISON_FOUND)
    add_custom_command(
            OUTPUT ${BISON_OUTPUT_C} ${BISON_OUTPUT_H}
            COMMAND ${BISON_EXECUTABLE}
            --output=${BISON_OUTPUT_C}
            -d
            ${CMAKE_CURRENT_SOURCE_DIR}/monga.y
            COMMENT "Generating monga.y.c and monga.y.h")
endif ()

set(FLEX_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/monga.l.c)
if (FLEX_FOUND)
    add_custom_command(
            OUTPUT ${FLEX_OUTPUT}
            COMMAND ${FLEX_EXECUTABLE}
            --outfile=${FLEX_OUTPUT}
            ${CMAKE_CURRENT_SOURCE_DIR}/monga.l
            COMMENT "Generating monga.l.c")
endif ()

add_library(monga_l STATIC
        ${FLEX_OUTPUT}
        ${BISON_OUTPUT_H})

target_link_libraries(monga_l monga_utils)

target_include_directories(monga_l PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR})

add_library(monga_y STATIC
        ${BISON_OUTPUT_C}
        ${BISON_OUTPUT_H})

target_link_libraries(monga_y monga_l monga_ast)

include(AddTester) # add_tester

add_subdirectory(ldb)
add_subdirectory(ydb)