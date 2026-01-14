add_executable(cxyier main.cpp)

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
)

target_include_directories(cxy_memory PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link memory library to main executable
target_link_libraries(cxyier PRIVATE cxy_memory)
