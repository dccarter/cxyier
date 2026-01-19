add_executable(cxy main.cpp)

# Create a library for the memory management system
add_library(cxy_memory STATIC
    memory/arena_allocator.cpp
    memory/stack_arena.cpp
    strings/strings.cpp
    diagnostics/diagnostics.cpp
    frontend/flags.cpp
    frontend/token.cpp
    frontend/lexer.cpp
    frontend/ast/visitor.cpp
    frontend/ast/printer.cpp
    frontend/parser.cpp
    types/type.cpp
    types/primitive.cpp
    types/composite.cpp
    types/utilities.cpp
    compiler/options.cpp
    compiler/option_utils.cpp
    compiler/option_parser.cpp
)

target_include_directories(cxy_memory PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link memory library to main executable
target_link_libraries(cxy PRIVATE cxy_memory)
