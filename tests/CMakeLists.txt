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
    parser/test_token_buffer.cpp
    parser/test_identifiers.cpp
    parser/test_primary_expressions.cpp
    parser/literals/test_integer_literals.cpp
    parser/literals/test_float_literals.cpp
    parser/literals/test_string_literals.cpp
    parser/literals/test_character_literals.cpp
    parser/literals/test_boolean_null_literals.cpp
    parser/expressions/test_arithmetic_expressions.cpp
    parser/expressions/test_shift_expressions.cpp
    parser/expressions/test_relational_expressions.cpp
    parser/expressions/test_equality_expressions.cpp
    parser/expressions/test_bitwise_expressions.cpp
    parser/expressions/test_logical_expressions.cpp
    parser/expressions/test_conditional_expressions.cpp
    parser/expressions/test_assignment_expressions.cpp
    parser/expressions/test_array_literals.cpp
    parser/expressions/test_tuple_literals.cpp
    parser/expressions/test_struct_literals.cpp
    parser/expressions/test_range_expressions.cpp
    parser/expressions/test_function_calls.cpp
    parser/expressions/test_indexing.cpp
    parser/expressions/test_member_access.cpp
    parser/expressions/test_cast_expressions.cpp
    parser/expressions/test_spread_expressions.cpp
    parser/expressions/test_string_interpolation.cpp
    parser/expressions/test_macro_calls.cpp
    parser/test_expression_integration.cpp
    parser/debug_test.cpp
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
