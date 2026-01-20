#pragma once

// Convenience header that includes all compiler stages

#include <cxy/stages/symbols.hpp>

namespace cxy {

// Stages convenience namespace
namespace stages {
    // Re-export main stage classes for easier access
    using symbols::Symbol;
    using symbols::Scope;
    using symbols::SymbolTable;
} // namespace stages

} // namespace cxy