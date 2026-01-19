#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace cxy::compiler {

/// Command types for the compiler
enum class Command : std::uint8_t {
    Dev,        ///< Development mode with debugging features
    Build,      ///< Standard compilation/build mode
    Test,       ///< Run internal tests
    Run,        ///< Compile and execute (future)
    Check,      ///< Syntax/semantic check only
    Help,       ///< Show help information
    Version     ///< Show version information
};

/// Compilation stages where the compiler can stop
enum class CompileStage : std::uint8_t {
    Lex,        ///< Stop after lexical analysis
    Parse,      ///< Stop after parsing
    Semantic,   ///< Stop after semantic analysis
    Optimize,   ///< Stop after optimization
    Codegen     ///< Complete compilation
};

/// Debug dump modes for various compiler outputs
enum class DumpMode : std::uint8_t {
    None,
    Tokens,     ///< Lexer output
    AST,        ///< Abstract syntax tree
    ASTJson,    ///< AST in JSON format
    ASTDebug,   ///< AST with debug information
    Diagnostics, ///< Diagnostic messages
    Memory,     ///< Memory usage statistics
    Timing      ///< Performance timing data
};

/// Diagnostic format options
enum class DiagnosticFormat : std::uint8_t {
    Default,    ///< Standard format
    Json,       ///< Machine-readable JSON
    Brief,      ///< Minimal output
    Verbose     ///< Detailed with context
};

/// Optimization levels
enum class OptimizationLevel : std::uint8_t {
    None,       ///< O0 - No optimization
    Basic,      ///< O1 - Basic optimizations
    Standard,   ///< O2 - Standard optimizations
    Aggressive  ///< O3 - Aggressive optimizations
};

/// Build target types
enum class BuildTarget : std::uint8_t {
    Executable, ///< Build executable binary
    Shared,     ///< Build shared library
    Static      ///< Build static library
};

/// Option categories for organization and help generation
enum class OptionCategory : std::uint8_t {
    Input,      ///< Source files, input formats
    Output,     ///< Output files, formats, stages
    Debug,      ///< Debugging and development features
    Optimization, ///< Performance optimizations
    Diagnostic, ///< Error reporting and verbosity
    Feature,    ///< Language feature controls
    System,     ///< Platform and environment settings
    Memory      ///< Memory management options
};

/// Command-specific options for development mode
struct DevOptions {
    bool printTokens = false;           ///< Print tokenized output
    bool printAST = false;              ///< Print abstract syntax tree
    bool emitDebugInfo = false;         ///< Include debug information
    bool cleanAST = false;              ///< Clean AST output (no metadata)
    bool withLocation = true;           ///< Include source location info
    bool withoutAttrs = false;          ///< Exclude attributes in output
    DumpMode dumpMode = DumpMode::None; ///< What to dump
    std::optional<std::filesystem::path> dumpFile; ///< File to dump output to
};

/// Command-specific options for build mode
struct BuildOptions {
    BuildTarget target = BuildTarget::Executable; ///< What to build
    bool noPIE = false;                           ///< Disable position independent executable
    std::vector<std::string> cflags;              ///< Additional C flags
    std::vector<std::string> libraries;           ///< Libraries to link against
};

/// Command-specific options for test mode
struct TestOptions {
    std::vector<std::string> testFilters;        ///< Test name filters
    bool verbose = false;                        ///< Verbose test output
    bool stopOnFirstFailure = false;             ///< Stop on first test failure
};

/// Debug and development options
struct DebugOptions {
    bool verbose = false;                        ///< Verbose output
    bool showTiming = false;                     ///< Show timing information
    bool showMemoryUsage = false;                ///< Show memory usage statistics
    bool preserveTemps = false;                  ///< Keep temporary files
    bool debugParser = false;                   ///< Enable parser debugging
    bool debugLexer = false;                    ///< Enable lexer debugging
};

/// Diagnostic reporting options
struct DiagnosticOptions {
    bool warningsAsErrors = false;               ///< Treat warnings as errors
    bool suppressWarnings = false;               ///< Suppress all warnings
    std::vector<std::string> disableWarnings;   ///< Specific warnings to disable
    std::vector<std::string> enableWarnings;    ///< Specific warnings to enable
    DiagnosticFormat format = DiagnosticFormat::Default; ///< Output format
    bool showColors = true;                      ///< Use colored output (auto-detect TTY)
    std::uint32_t maxErrors = 100;               ///< Maximum number of errors before stopping
};

/// Language feature control options
struct FeatureOptions {
    bool enableExperimentalFeatures = false;    ///< Enable experimental language features
    std::vector<std::string> enabledFeatures;   ///< Explicitly enabled features
    std::vector<std::string> disabledFeatures;  ///< Explicitly disabled features
    
    // Phase 1 specific options
    bool strictNumberLiterals = false;          ///< Strict number literal parsing
    bool allowUnicodeIdentifiers = true;        ///< Allow Unicode in identifiers
};

/// Optimization control options
struct OptimizationOptions {
    OptimizationLevel level = OptimizationLevel::None; ///< Optimization level
    bool debugInfo = false;                            ///< Include debug information
    std::string passes;                                ///< Custom optimization passes
    bool debugPassManager = false;                     ///< Debug pass execution
};

/// System and environment options
struct SystemOptions {
    std::optional<std::filesystem::path> stdlib;        ///< Standard library override
    std::vector<std::filesystem::path> includePaths;    ///< Include search paths
    std::optional<std::string> targetTriple;            ///< Target architecture triple
    std::optional<std::filesystem::path> sysroot;       ///< System root directory
    
    // Core directories
    std::optional<std::filesystem::path> libDir;        ///< Standard library directory
    std::optional<std::filesystem::path> buildDir;      ///< Build output directory
    std::optional<std::filesystem::path> pluginsDir;    ///< Compiler plugins directory
    
    // Search paths
    std::vector<std::filesystem::path> librarySearchPaths;  ///< Library search paths (-L)
    std::vector<std::filesystem::path> frameworkSearchPaths; ///< Framework search paths (future)
    
    // System configuration
    std::optional<std::string> operatingSystem;         ///< Target operating system override
    bool buildPlugin = false;                           ///< Build as compiler plugin
};

/// Memory management options
struct MemoryOptions {
    std::size_t arenaSize = 64 * 1024 * 1024;          ///< Arena size in bytes (64MB default)
    bool enableMemoryTracking = false;                  ///< Enable memory usage tracking
    bool showArenaStats = false;                        ///< Show arena allocation statistics
    bool withMemoryTrace = false;                       ///< Enable memory tracing
};

} // namespace cxy::compiler