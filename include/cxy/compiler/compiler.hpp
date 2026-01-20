#pragma once

#include <cxy/compiler/compilation_result.hpp>
#include <cxy/compiler/module_cache.hpp>
#include <cxy/compiler/options.hpp>
#include <cxy/ast/node.hpp>
#include <cxy/diagnostics.hpp>
#include <cxy/memory/arena.hpp>
#include <cxy/strings.hpp>
#include <cxy/types/registry.hpp>
#include <filesystem>
#include <memory>
#include <string_view>

// Forward declarations
namespace cxy {
    class Lexer;
    class Parser;
    class SourceManager;
}

namespace cxy::compiler {

/**
 * @brief Main compiler class that orchestrates all compilation phases.
 *
 * The Compiler class is the primary interface for compiling Cxyier source code.
 * It coordinates between all compiler subsystems including lexing, parsing,
 * semantic analysis, and code generation while managing memory, diagnostics,
 * and module imports.
 *
 * Key responsibilities:
 * - Orchestrate compilation pipeline execution
 * - Manage global type registry for type consistency
 * - Handle module import resolution and caching
 * - Coordinate arena-based memory management
 * - Integrate diagnostic reporting across all phases
 * - Provide high-level compilation interface
 *
 * Design principles:
 * - Single Responsibility: Focus on orchestration, delegate work to specialists
 * - Configuration-Driven: All behavior controlled through CompilerOptions
 * - Fail-Fast: Stop on fatal errors while collecting non-fatal diagnostics
 * - Arena-Based: Predictable memory management with automatic cleanup
 */
class Compiler {
public:
    /**
     * @brief Create compiler with specified configuration.
     *
     * @param options Compiler configuration and behavior settings
     * @param projectRoot Root directory for the project (security boundary for relative imports)
     */
    explicit Compiler(CompilerOptions options, 
                     std::filesystem::path projectRoot = std::filesystem::current_path());

    /**
     * @brief Destructor.
     *
     * Automatically cleans up arena-allocated memory and cached modules.
     */
    ~Compiler();

    // Disable copying (compiler maintains significant state)
    Compiler(const Compiler&) = delete;
    Compiler& operator=(const Compiler&) = delete;

    // Disable moving (TypeRegistry doesn't support moving)
    Compiler(Compiler&&) = delete;
    Compiler& operator=(Compiler&&) = delete;

    /**
     * @brief Compile a single source file.
     *
     * Reads the file from disk and runs the complete compilation pipeline.
     * Handles include processing and module imports automatically.
     *
     * @param sourcePath Path to source file to compile
     * @return CompilationResult with status, AST, and diagnostic counts
     */
    [[nodiscard]] CompilationResult compileFile(const std::filesystem::path& sourcePath);

    /**
     * @brief Compile source code from string.
     *
     * Compiles source code provided as a string with an optional filename
     * for diagnostic reporting. Useful for testing and interactive compilation.
     *
     * @param source Source code to compile
     * @param filename Filename for diagnostic reporting (default: "<input>")
     * @return CompilationResult with status, AST, and diagnostic counts
     */
    [[nodiscard]] CompilationResult compileSource(std::string_view source, 
                                                  std::string_view filename = "<input>");

    /**
     * @brief Compile source code from string (convenience alias).
     *
     * Identical to compileSource, provided for API compatibility.
     *
     * @param source Source code to compile
     * @param filename Filename for diagnostic reporting (default: "<input>")
     * @return CompilationResult with status, AST, and diagnostic counts
     */
    [[nodiscard]] CompilationResult compileString(std::string_view source, 
                                                  std::string_view filename = "<input>");

    /**
     * @brief Import module during compilation.
     *
     * Called by parser during import resolution. Runs full compilation
     * pipeline including semantic analysis to provide complete type
     * information for the importing module.
     *
     * @param modulePath Import path from source code (e.g., "./utils.cxy", "stdlib/io.cxy")
     * @param currentFile Path of file containing the import (for relative resolution)
     * @param importLocation Location of import statement for diagnostics
     * @return Parsed and semantically analyzed AST, or nullptr on failure
     */
    [[nodiscard]] ast::ASTNode* importModule(
        std::string_view modulePath, 
        const std::filesystem::path& currentFile,
        const Location& importLocation);

    /**
     * @brief Get reference to the global type registry.
     *
     * The type registry is shared across all compilation units to ensure
     * type consistency and enable efficient type reuse.
     *
     * @return Reference to the compiler's type registry instance
     */
    [[nodiscard]] TypeRegistry& getTypeRegistry() { 
        return typeRegistry_; 
    }

    /**
     * @brief Get const reference to the global type registry.
     *
     * @return Const reference to the compiler's type registry instance
     */
    [[nodiscard]] const TypeRegistry& getTypeRegistry() const { 
        return typeRegistry_; 
    }

    /**
     * @brief Get reference to the diagnostic logger.
     *
     * Provides access to the diagnostic system for error reporting
     * and diagnostic collection.
     *
     * @return Reference to the diagnostic logger
     */
    [[nodiscard]] DiagnosticLogger& getDiagnostics() { 
        return diagnostics_; 
    }

    /**
     * @brief Get const reference to the diagnostic logger.
     *
     * @return Const reference to the diagnostic logger
     */
    [[nodiscard]] const DiagnosticLogger& getDiagnostics() const { 
        return diagnostics_; 
    }

    /**
     * @brief Get reference to the source manager.
     *
     * Provides access to source file management and line/column
     * resolution for diagnostics.
     *
     * @return Reference to the source manager
     */
    [[nodiscard]] SourceManager& getSourceManager() { 
        return sourceManager_; 
    }

    /**
     * @brief Get const reference to the source manager.
     *
     * @return Const reference to the source manager
     */
    [[nodiscard]] const SourceManager& getSourceManager() const { 
        return sourceManager_; 
    }

    /**
     * @brief Get reference to the string interner.
     *
     * Provides access to string interning for efficient string
     * storage and comparison.
     *
     * @return Reference to the string interner
     */
    [[nodiscard]] StringInterner& getStringInterner() { 
        return stringInterner_; 
    }

    /**
     * @brief Get const reference to the string interner.
     *
     * @return Const reference to the string interner
     */
    [[nodiscard]] const StringInterner& getStringInterner() const { 
        return stringInterner_; 
    }

    /**
     * @brief Get the compiler options.
     *
     * @return Const reference to the compiler configuration
     */
    [[nodiscard]] const CompilerOptions& getOptions() const { 
        return options_; 
    }

    /**
     * @brief Get the project root directory.
     *
     * @return Project root path (security boundary for relative imports)
     */
    [[nodiscard]] const std::filesystem::path& getProjectRoot() const { 
        return projectRoot_; 
    }

    /**
     * @brief Get the module cache.
     *
     * @return Reference to the module cache for import management
     */
    [[nodiscard]] ModuleCache& getModuleCache() { 
        return moduleCache_; 
    }

    /**
     * @brief Get the module cache (const version).
     *
     * @return Const reference to the module cache
     */
    [[nodiscard]] const ModuleCache& getModuleCache() const { 
        return moduleCache_; 
    }

private:
    // Configuration and paths
    CompilerOptions options_;               ///< Compiler configuration
    std::filesystem::path projectRoot_;     ///< Project root directory (security boundary)

    // Core subsystems
    TypeRegistry typeRegistry_;             ///< Global type registry for all compilation units
    DiagnosticLogger diagnostics_;          ///< Diagnostic reporting system
    SourceManager sourceManager_;           ///< Source file management
    ArenaAllocator arena_;                  ///< Arena allocator for AST nodes
    StringInterner stringInterner_;         ///< String interning for efficiency

    // Module import management
    ModuleCache moduleCache_;               ///< Module caching and import cycle detection

    /**
     * @brief Print AST for debugging based on DevOptions configuration.
     *
     * Uses the current DevOptions settings to determine output format and destination.
     * Supports clean output, location info, and attribute inclusion based on flags.
     *
     * @param ast AST node to print
     */
    void printASTDebug(const ast::ASTNode* ast) const;

    /**
     * @brief Resolve module path for import.
     *
     * Handles both relative imports (./file.cxy) and library imports (stdlib/io.cxy).
     * Performs security validation to prevent directory traversal attacks.
     *
     * @param modulePath Import path from source code
     * @param currentFile File containing the import statement
     * @return Resolved absolute path, or empty optional on failure
     */
    [[nodiscard]] std::optional<std::filesystem::path> resolveModulePath(
        std::string_view modulePath,
        const std::filesystem::path& currentFile) const;

    /**
     * @brief Get the effective library directory from options.
     *
     * @return Library directory path for resolving library imports
     */
    [[nodiscard]] std::filesystem::path getEffectiveLibDir() const;

    /**
     * @brief Load and compile a module.
     *
     * Reads source file, runs compilation pipeline, and caches result.
     * Performs semantic analysis to provide complete type information.
     *
     * @param modulePath Canonical path to module file
     * @param importLocation Location of import statement for diagnostics
     * @return Compiled AST with semantic information, or nullptr on failure
     */
    [[nodiscard]] ast::ASTNode* loadModule(
        const std::filesystem::path& modulePath,
        const Location& importLocation);

    /**
     * @brief Run the compilation pipeline on source code.
     *
     * Executes lexing, parsing, and optional semantic analysis phases.
     *
     * @param source Source code to compile
     * @param filename Filename for diagnostics
     * @param runSemanticAnalysis Whether to run semantic analysis pass
     * @return CompilationResult with status and AST
     */
    [[nodiscard]] CompilationResult runCompilationPipeline(
        std::string_view source,
        std::string_view filename,
        bool runSemanticAnalysis = false);


};

} // namespace cxy::compiler