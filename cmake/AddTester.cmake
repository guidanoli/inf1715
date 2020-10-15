cmake_minimum_required(VERSION 3.0)

# Add a tester
# add_tester(tester tester_dir)
# Note: prior to CMake version 3.19, test names cannot
# contain special characters such as whitespaces or quotation marks
function(add_tester tester tester_dir)
    add_test(
        NAME ${tester}
        COMMAND ${BASH} tests/run ${tester} ${tester_dir}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
endfunction()
