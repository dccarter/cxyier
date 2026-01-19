#pragma once

/// @file compiler.hpp
/// @brief Main header for the Cxy compiler option system
///
/// This header provides a convenient single include for all compiler
/// option-related functionality, including option definitions, parsing,
/// and validation.

#include "compiler/options_types.hpp"
#include "compiler/options.hpp"
#include "compiler/option_parser.hpp"

/// @namespace cxy::compiler
/// @brief Namespace containing all compiler option system components
///
/// This namespace provides:
/// - CompilerOptions: Main options class with all configuration
/// - OptionParser: Command-line and configuration file parsing
/// - Various enums and option structures for type-safe configuration
///
/// @example Basic usage:
/// ```cpp
/// #include <cxy/compiler.hpp>
/// 
/// int main(int argc, char** argv) {
///     cxy::DiagnosticLogger diagnostics;
///     cxy::compiler::OptionParser parser(diagnostics);
///     cxy::compiler::CompilerOptions options;
///     
///     auto result = parser.parseCommandLine(argc, argv, options);
///     if (result == cxy::compiler::ParseResult::Success) {
///         // Use options for compilation
///     }
///     return 0;
/// }
/// ```
namespace cxy::compiler {

/// @brief Version information for the compiler option system
struct VersionInfo {
    static constexpr int MAJOR = 0;     ///< Major version number
    static constexpr int MINOR = 1;     ///< Minor version number
    static constexpr int PATCH = 0;     ///< Patch version number
    static constexpr const char* PHASE = "Phase1"; ///< Development phase
};

} // namespace cxy::compiler