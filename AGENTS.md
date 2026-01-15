# AI Agent Instructions for Cxyier Programming Language

## Project Overview

Cxyier is a modern compiled programming language implemented in C++23. This project uses a carefully designed architecture with arena-based memory management, comprehensive diagnostic reporting, and a multi-phase parser development approach.

## Development Philosophy

### Modern C++23 Standards

- **Always use modern C++23 features** where appropriate
- Prefer standard library containers and algorithms over hand-rolled solutions
- Use `std::format` for string formatting instead of printf-style formatting
- Leverage auto type deduction, range-based for loops, and structured bindings
- Use `[[nodiscard]]` for functions that return important values
- Prefer `enum class` over plain enums
- Use RAII principles consistently

### Incremental Development

- **Feature development must be incremental and unit-testable**
- Each component should be implementable and testable in isolation
- Follow the established phase-based approach (see Parser phases in `docs/stage/PARSER.md`)
- New features require corresponding test cases before implementation
- Avoid large monolithic changes - break work into small, verifiable steps

### Code Quality Standards

- Write self-documenting code with clear variable and function names
- Add comprehensive documentation for public APIs
- Use consistent naming conventions:
  - `PascalCase` for classes and types
  - `camelCase` for functions and variables
  - `SCREAMING_SNAKE_CASE` for constants
  - `snake_case_` with trailing underscore for private members
- Prefer composition over inheritance
- Use smart pointers where appropriate, arena allocation where specified

## Project Architecture

### Build System (CMake)

- **Language Standard**: C++23 (required)
- **Minimum CMake**: 3.20+
- **Build Commands**:
  ```bash
  mkdir build && cd build
  cmake ..
  make                    # Build all targets
  make cxyier_tests      # Build tests
  ctest                  # Run tests
  ./build/tests/cxyier_tests # Run tests
  ```

### Directory Structure

```
cxyier/
├── include/cxy/          # Public headers
├── src/                  # Implementation files
│   ├── memory/          # Arena allocators
│   ├── strings/         # String interning
│   ├── diagnostics/     # Error reporting
│   └── frontend/        # Lexer, parser, AST
├── tests/               # Comprehensive test suite
├── docs/stage/          # Development documentation
└── third_party/         # External dependencies (Catch2)
```

### Core Components

#### Memory Management

- **Arena Allocation**: Primary memory strategy using `ArenaAllocator`
- **String Interning**: All identifiers and string literals are interned
- **RAII**: Automatic cleanup, no manual memory management in user code
- **Performance**: Stack-first allocation with heap fallback for large objects

#### Lexing & Parsing

- **Lexer**: Token-based with comprehensive escape sequence processing
- **Parser**: LL(3) recursive descent with 4-token sliding window
- **Error Recovery**: Sophisticated diagnostic reporting and synchronization
- **Phased Development**: Currently in Phase 1 (literals and identifiers)

#### Diagnostics

- **Structured Reporting**: Location-aware error messages
- **Multiple Sinks**: Console output, in-memory collection for tests
- **Severity Levels**: Info, Warning, Error, Fatal
- **Source Integration**: Rich context with source line display

## Testing Requirements

### Test Organization

- **Unit Tests**: Located in `tests/` directory
- **Framework**: Catch2 (single header in `third_party/`)
- **Coverage**: Every public API must have corresponding tests
- **Naming**: Test files follow pattern `test_*.cpp` or `*_test.cpp`

### Test Categories

1. **Component Tests**: Individual class/function testing
2. **Integration Tests**: Cross-component interaction
3. **Error Handling**: Verify diagnostic reporting works correctly
4. **Performance Tests**: Arena allocation efficiency, parsing speed
5. **Edge Cases**: Boundary conditions, malformed input handling

### Test Patterns

```cpp
// Use test utilities for consistent setup
auto fixture = createParserFixture("test source code");
auto result = fixture->parseExpression();

// Use descriptive expectations
expectIntegerLiteral(result, 42);
expectParseFailure(result);  // For error cases

// Use floating-point comparison helpers
REQUIRE(value == Catch::Approx(expected));
```

## Working with Existing Systems

### Parser Development

- **Current Phase**: Phase 1 (literals, identifiers, parenthesized expressions)
- **Token Buffer**: 4-element sliding window [previous, current, lookahead1, lookahead2]
- **Error Integration**: All parse errors go through `DiagnosticLogger`
- **AST Creation**: Uses arena allocation for all nodes

### Lexer Integration

- **String Processing**: Escape sequences processed during lexing
- **Value Storage**: Tokens contain processed values (not raw text)
- **Interning**: Identifiers and strings are interned at lex time
- **Performance**: Stack buffer (512 bytes) with heap fallback for large strings

### Adding New Features

#### 1. Design Phase

- Review existing documentation in `docs/stage/`
- Identify dependencies and integration points
- Design for testability (small, focused components)
- Consider memory management implications

#### 2. Implementation Phase

- Start with comprehensive tests (TDD approach)
- Implement incrementally with frequent compilation
- Follow established patterns in existing code
- Use modern C++23 features appropriately

#### 3. Integration Phase

- Update relevant documentation
- Verify no regressions in existing tests
- Add integration tests for cross-component features
- Update CMakeLists.txt if new files are added

## Common Patterns

### Arena-Allocated Objects

```cpp
// Creation
auto* node = ast::createIntLiteral(value, location, arena);

// Storage - use raw pointers for arena objects
ast::ASTNode* expression;  // OK - arena manages lifetime

// Never delete arena objects manually
```

### Error Handling

```cpp
// Parser errors
if (!expect(TokenKind::RParen)) {
    return nullptr;  // Error already reported
}

// Diagnostic integration
diagnostics.error("Error message", location);
diagnostics.warning(location, "Format string: {}", value);
```

### String Handling

```cpp
// Use interned strings for identifiers/literals
InternedString name = token.value.stringValue;

// Use string_view for temporary operations
std::string_view processText(std::string_view input);
```

## Performance Considerations

### Memory Efficiency

- Prefer arena allocation over individual heap allocations
- Use string views instead of copying strings unnecessarily
- Minimize token copying in parser buffer operations
- Cache commonly used parsing results when beneficial

### Compilation Speed

- Use forward declarations in headers when possible
- Avoid unnecessary template instantiations
- Keep header dependencies minimal
- Use precompiled headers for stable dependencies if needed

## Error Guidelines

### For AI Agents

- **Always compile-test changes** before submitting
- **Run relevant tests** to verify functionality
- **Check diagnostics output** to ensure no warnings introduced
- **Follow existing code style** rather than imposing different patterns
- **Ask for clarification** rather than guessing about unclear requirements

### Common Pitfalls

- Don't use arena allocation for temporary buffers in lexer
- Don't manually delete arena-allocated objects
- Don't bypass the diagnostic system for error reporting
- Don't break the incremental development approach with large changes
- Don't ignore existing test patterns and utilities

## Documentation Standards

### Code Documentation

- Document all public APIs with comprehensive comments
- Include usage examples for complex interfaces
- Explain design decisions for non-obvious implementations
- Update documentation when changing existing interfaces

### Architecture Documentation

- Keep `docs/stage/*.md` files updated with design changes
- Document new patterns that could be reused
- Explain integration points between components
- Maintain accurate phase information for development tracking

---

## Getting Started

1. **Read the relevant design documents** in `docs/stage/` before starting work
2. **Build and run tests** to understand current system state
3. **Study existing test patterns** to follow established conventions
4. **Start with unit tests** before implementing new functionality
5. **Use incremental development** - small, verifiable steps are preferred
6. **Integrate diagnostics properly** - never ignore error reporting requirements

Remember: This is a language implementation project that values correctness, performance, and maintainability. Every change should move us closer to a robust, well-tested compiler infrastructure.
