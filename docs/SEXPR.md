# S-Expression Format for AST Testing

## Overview

S-expressions provide a human-readable representation of AST nodes for testing and debugging. They follow a simple Lisp-like format: `(NodeType [data] [children...])`

## Basic Structure

### Node Format

```
(NodeKind value children...)
```

- **NodeKind**: AST node type without "ast" prefix (Int, BinaryExpr, Identifier)
- **value**: Node-specific data (operators, literals, names)
- **children**: Child nodes as nested S-expressions

### Examples

```
(Int 42)                                           // Integer literal
(String "hello")                                   // String literal
(Identifier variable)                              // Identifier
(BinaryExpr + (Int 2) (Int 3))                    // Binary operation
(UnaryExpr - (Int 5))                             // Unary operation
(AssignmentExpr (Identifier x) = (Int 10))        // Assignment
```

## Printer Configuration

S-expression output varies based on `PrinterConfig` flags:

### Available Flags

```cpp
enum class PrinterFlags {
  None = 0,                    // Minimal output
  IncludeLocation = 1 << 0,    // Add source location info
  IncludeTypes = 1 << 1,       // Add type annotations
  IncludeFlags = 1 << 2,       // Add node flags
  IncludeMetadata = 1 << 3,    // Add metadata info
  IncludeAttributes = 1 << 4,  // Add node attributes
  CompactLiterals = 1 << 5,    // Compact literal representation
  CompactMode = 1 << 6,        // Single line, minimal whitespace
  Default = IncludeLocation    // Default configuration
};
```

### Configuration Examples

#### Basic Output (None)

```cpp
PrinterConfig config{PrinterFlags::None};
```

Output: `(BinaryExpr + (Int 2) (Int 3))`

#### With Location Information (Default)

```cpp
PrinterConfig config{PrinterFlags::IncludeLocation};
// or
PrinterConfig config{PrinterFlags::Default};
```

Output: `(BinaryExpr @1:5-1:9 + (Int @1:5 2) (Int @1:7 3))`

#### With Type Information

```cpp
PrinterConfig config{PrinterFlags::IncludeTypes};
```

Output: `(BinaryExpr [type=Type] + (Int [type=Type] 2) (Int [type=Type] 3))`

#### With Node Flags

```cpp
PrinterConfig config{PrinterFlags::IncludeFlags};
```

Output: `(BinaryExpr [flags=const] + (Int 2) (Int 3))`

#### With Metadata

```cpp
PrinterConfig config{PrinterFlags::IncludeMetadata};
```

Output: `(BinaryExpr [metadata=2 entries] + (Int 2) (Int 3))`

#### With Attributes

```cpp
PrinterConfig config{PrinterFlags::IncludeAttributes};
```

Output: `(BinaryExpr [attr1 attr2(param1 param2)] + (Int 2) (Int 3))`

#### Compact Mode

```cpp
PrinterConfig config{PrinterFlags::CompactMode};
```

Output: `(BinaryExpr + (Int 2) (Int 3))` - single line, no indentation

#### Combined Flags

```cpp
PrinterConfig config{PrinterFlags::IncludeLocation | PrinterFlags::IncludeTypes};
```

Output: `(BinaryExpr @1:5 [type=Type] + (Int @1:5 [type=Type] 2) (Int @1:7 [type=Type] 3))`

### Additional Configuration

```cpp
struct PrinterConfig {
  PrinterFlags flags = PrinterFlags::Default;
  uint32_t maxDepth = 0;       // 0 = unlimited depth
  uint32_t indentSize = 2;     // Spaces per indent level
  std::string_view nodePrefix; // Optional prefix for node names
  std::function<bool(const ASTNode*)> nodeFilter; // Node filtering
};
```

#### Depth Limiting

```cpp
PrinterConfig config{PrinterFlags::None};
config.maxDepth = 2;
```

Deep nodes show as `...`

#### Custom Indentation

```cpp
PrinterConfig config{PrinterFlags::None};
config.indentSize = 4;  // 4 spaces per level
```

#### Node Filtering

```cpp
PrinterConfig config{PrinterFlags::None};
config.nodeFilter = [](const ASTNode* node) {
  return node->kind != astNoop;  // Skip noop nodes
};
```

## Testing Usage

### Test Macros

#### `REQUIRE_AST_MATCHES(node, expected)`

Exact string matching with whitespace normalization:

```cpp
auto* expr = parseExpression("2 + 3");
REQUIRE_AST_MATCHES(expr, "(BinaryExpr + (Int 2) (Int 3))");
```

#### `REQUIRE_AST_STRUCTURALLY_MATCHES(node, expected)`

Structural comparison, robust against formatting:

```cpp
REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
  (BinaryExpr +
    (Int 2)
    (Int 3))
)");
```

### Whitespace Handling

All these formats are equivalent:

```cpp
"(BinaryExpr + (Int 2) (Int 3))"
"( BinaryExpr   +   ( Int 2 )   ( Int 3 ) )"
R"(
(BinaryExpr +
  (Int 2)
  (Int 3))
)"
```

## Common Node Types

### Literals

- `(Int 42)` - Integer
- `(Float 3.14)` - Float
- `(String "text")` - String
- `(Char 'x')` - Character
- `(Bool true)` - Boolean
- `(Null)` - Null

### Expressions

- `(BinaryExpr op left right)` - Binary operations
- `(UnaryExpr op operand)` - Prefix unary
- `(UnaryExpr operand op [postfix])` - Postfix unary
- `(AssignmentExpr target op value)` - Assignments

### Complex Examples

```cpp
// Precedence: 2 + 3 * 4
(BinaryExpr +
  (Int 2)
  (BinaryExpr *
    (Int 3)
    (Int 4)))

// Assignment: x += y * 2
(AssignmentExpr (Identifier x) +=
  (BinaryExpr *
    (Identifier y)
    (Int 2)))
```

## Best Practices

1. Use raw strings (`R"(...)"`) for multi-line S-expressions
2. Indent child nodes by 2 spaces
3. Use `REQUIRE_AST_STRUCTURALLY_MATCHES` for complex expressions
4. Group related tests by operator type or precedence

## Attribute Format Details

Attributes are printed inside square brackets with this format:

- `[attr1 attr2]` - Simple attributes without parameters
- `[attr1(arg1 arg2) attr2]` - Attributes with positional parameters
- `[attr1(name value) attr2]` - Attributes with named parameters
- Arguments are printed as their literal values (strings, numbers, identifiers)
