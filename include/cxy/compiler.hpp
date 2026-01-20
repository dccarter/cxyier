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
#include "compiler/compilation_result.hpp"
#include "compiler/compiler.hpp"
#include "compiler/module_cache.hpp"

/// @namespace cxy::compiler
/// @brief Namespace containing all compiler system components
///
/// This namespace provides:
/// - Compiler: Main compiler orchestration class
/// - CompilerOptions: Configuration and behavior settings
/// - CompilationResult: Compilation outcome and diagnostic information
/// - OptionParser: Command-line and configuration file parsing
/// - ModuleCache: Module import caching and cycle detection
/// - Various enums and option structures for type-safe configuration
///
/// @example Basic compilation:
/// ```cpp
/// #include <cxy/compiler.hpp>
/// 
/// int main(int argc, char** argv) {
///     cxy::DiagnosticLogger diagnostics;
///     cxy::compiler::OptionParser parser(diagnostics);
///     cxy::compiler::CompilerOptions options;
///     
///     auto parseResult = parser.parseCommandLine(argc, argv, options);
///     if (parseResult == cxy::compiler::ParseResult::Success) {
///         cxy::compiler::Compiler compiler(std::move(options));
///         auto result = compiler.compileFile("main.cxy");
///         if (result.isSuccess()) {
///             // Compilation successful
///         }
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