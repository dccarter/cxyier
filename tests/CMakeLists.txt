# Create a library for Catch2 single-header
add_library(catch2_main ${CMAKE_SOURCE_DIR}/third_party/catch2.cpp)
target_include_directories(catch2_main PUBLIC ${CMAKE_SOURCE_DIR}/third_party)
target_compile_features(catch2_main PUBLIC cxx_std_17)

# Test executable
add_executable(cxyier_tests
    test_arena_allocator.cpp
    test_strings.cpp
    test_diagnostics.cpp
    test_token.cpp
    test_lexer.cpp
    test_string_interpolation.cpp
    test_includes.cpp
    test_ast_visitor.cpp
    test_ast_printer.cpp
)

# Link libraries (using single-header versions and our memory library)
target_link_libraries(cxyier_tests
    PRIVATE
    catch2_main
    cxy_memory
)

# Include directories for tests
target_include_directories(cxyier_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/third_party
)

# Register tests with CTest
include(CTest)

# Add custom test target
add_custom_target(test-verbose
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS cxyier_tests
)

# Manual test discovery
add_test(NAME cxyier_tests COMMAND cxyier_tests)
