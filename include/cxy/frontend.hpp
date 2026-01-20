#pragma once

// Convenience header that includes all frontend components

#include <cxy/frontend/lexer.hpp>
#include <cxy/frontend/parser.hpp>

namespace cxy {

// Frontend convenience namespace
namespace frontend {
    // Re-export main frontend classes for easier access
    using Lexer = cxy::Lexer;
    using Parser = cxy::Parser;
} // namespace frontend

} // namespace cxy