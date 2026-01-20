#pragma once

#include <cxy/ast/node.hpp>
#include <cxy/diagnostics.hpp>
#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace cxy::compiler {

/**
 * @brief Information about a cached compiled module.
 *
 * Contains the compiled AST along with metadata about the compilation
 * process and semantic analysis results.
 */
struct CachedModule {
    ast::ASTNode* ast;                          ///< Compiled and semantically analyzed AST (arena-allocated)
    std::filesystem::path canonicalPath;        ///< Canonical path to source file
    std::filesystem::file_time_type timestamp;  ///< File modification time when cached
    size_t errorCount = 0;                      ///< Number of errors during compilation
    size_t warningCount = 0;                    ///< Number of warnings during compilation
    bool hasSemanticInfo = false;               ///< Whether semantic analysis was completed

    /**
     * @brief Check if cached module is valid and up-to-date.
     * @return true if module is valid and file hasn't been modified
     */
    [[nodiscard]] bool isValid() const;

    /**
     * @brief Check if module was compiled successfully.
     * @return true if no errors and AST is available
     */
    [[nodiscard]] bool isSuccessful() const {
        return ast != nullptr && errorCount == 0;
    }
};

/**
 * @brief Cache for compiled modules to avoid redundant compilation.
 *
 * The ModuleCache manages the lifetime and validity of compiled modules,
 * enabling efficient reuse of imported modules across compilation units.
 * It tracks module dependencies, detects circular imports, and handles
 * cache invalidation based on file modification times.
 *
 * Key features:
 * - Canonical path-based caching to handle symbolic links
 * - Circular import detection with detailed error reporting
 * - File modification time tracking for cache invalidation
 * - Memory-efficient storage with arena allocation compatibility
 * - Thread-safe operations for future parallel compilation support
 */
class ModuleCache {
public:
    /**
     * @brief Default constructor.
     */
    ModuleCache() = default;

    /**
     * @brief Destructor.
     */
    ~ModuleCache() = default;

    // Disable copying (cache contains unique_ptr)
    ModuleCache(const ModuleCache&) = delete;
    ModuleCache& operator=(const ModuleCache&) = delete;

    // Allow moving
    ModuleCache(ModuleCache&&) = default;
    ModuleCache& operator=(ModuleCache&&) = default;

    /**
     * @brief Check if module is cached and valid.
     *
     * @param modulePath Canonical path to module file
     * @return true if module is cached and up-to-date
     */
    [[nodiscard]] bool isCached(const std::filesystem::path& modulePath) const;

    /**
     * @brief Get cached module if available and valid.
     *
     * @param modulePath Canonical path to module file
     * @return Pointer to cached AST, or nullptr if not cached/invalid
     */
    [[nodiscard]] const ast::ASTNode* getCachedModule(const std::filesystem::path& modulePath) const;

    /**
     * @brief Cache a compiled module.
     *
     * Stores the compiled AST with metadata for future reuse.
     * The AST must be arena-allocated and will be managed by the arena.
     *
     * @param modulePath Canonical path to module file
     * @param ast Compiled AST (arena-allocated)
     * @param errorCount Number of compilation errors
     * @param warningCount Number of compilation warnings
     * @param hasSemanticInfo Whether semantic analysis was completed
     * @return true if successfully cached, false on error
     */
    [[nodiscard]] bool cacheModule(const std::filesystem::path& modulePath,
                                   ast::ASTNode* ast,
                                   size_t errorCount = 0,
                                   size_t warningCount = 0,
                                   bool hasSemanticInfo = false);

    /**
     * @brief Remove module from cache.
     *
     * @param modulePath Canonical path to module file
     * @return true if module was cached and removed
     */
    [[nodiscard]] bool removeModule(const std::filesystem::path& modulePath);

    /**
     * @brief Clear all cached modules.
     */
    void clear();

    /**
     * @brief Get cache statistics.
     *
     * @return Number of currently cached modules
     */
    [[nodiscard]] size_t size() const {
        return cache_.size();
    }

    /**
     * @brief Check if cache is empty.
     *
     * @return true if no modules are cached
     */
    [[nodiscard]] bool empty() const {
        return cache_.empty();
    }

    /**
     * @brief Begin import of a module (for cycle detection).
     *
     * Adds module to the current import stack to detect circular dependencies.
     * Must be paired with endImport() call.
     *
     * @param modulePath Canonical path to module being imported
     * @return true if import can proceed, false if would create cycle
     */
    [[nodiscard]] bool beginImport(const std::filesystem::path& modulePath);

    /**
     * @brief End import of a module (for cycle detection).
     *
     * Removes module from the current import stack.
     * Must be called after corresponding beginImport() call.
     *
     * @param modulePath Canonical path to module that was imported
     */
    void endImport(const std::filesystem::path& modulePath);

    /**
     * @brief Check if importing a module would create a circular dependency.
     *
     * @param modulePath Canonical path to module
     * @return true if importing would create a cycle
     */
    [[nodiscard]] bool wouldCreateCycle(const std::filesystem::path& modulePath) const;

    /**
     * @brief Get the current import stack.
     *
     * Returns the chain of modules currently being imported, useful for
     * detailed error reporting when circular imports are detected.
     *
     * @return Vector of module paths in import order
     */
    [[nodiscard]] std::vector<std::filesystem::path> getImportStack() const;

    /**
     * @brief Invalidate cached module if file has been modified.
     *
     * Checks file modification time and removes from cache if changed.
     *
     * @param modulePath Canonical path to module file
     * @return true if module was invalidated and removed
     */
    [[nodiscard]] bool invalidateIfModified(const std::filesystem::path& modulePath);

    /**
     * @brief Invalidate all cached modules that have been modified.
     *
     * Checks all cached modules against their file modification times
     * and removes any that have been changed.
     *
     * @return Number of modules that were invalidated
     */
    [[nodiscard]] size_t invalidateModified();

    /**
     * @brief Get detailed information about a cached module.
     *
     * @param modulePath Canonical path to module file
     * @return Cached module information, or nullptr if not cached
     */
    [[nodiscard]] const CachedModule* getModuleInfo(const std::filesystem::path& modulePath) const;

    /**
     * @brief Check if all cached modules have semantic information.
     *
     * @return true if all cached modules completed semantic analysis
     */
    [[nodiscard]] bool allModulesHaveSemanticInfo() const;

private:
    /// Map from canonical file path to cached module
    std::unordered_map<std::filesystem::path, CachedModule> cache_;

    /// Vector of modules currently being imported (for cycle detection and order)
    std::vector<std::filesystem::path> importStack_;

    /**
     * @brief Get file modification time safely.
     *
     * @param path File path to check
     * @return File modification time, or nullopt on error
     */
    [[nodiscard]] std::optional<std::filesystem::file_time_type> getFileTime(
        const std::filesystem::path& path) const;
};

/**
 * @brief RAII helper for import stack management.
 *
 * Automatically manages beginImport/endImport calls to ensure
 * proper cleanup even if exceptions occur during module compilation.
 */
class ImportGuard {
public:
    /**
     * @brief Begin import with automatic cleanup.
     *
     * @param cache Module cache to manage
     * @param modulePath Module being imported
     * @param diagnostics Diagnostic logger for error reporting
     */
    ImportGuard(ModuleCache& cache, 
                const std::filesystem::path& modulePath,
                DiagnosticLogger& diagnostics);

    /**
     * @brief Destructor automatically ends import.
     */
    ~ImportGuard();

    // Disable copying and moving
    ImportGuard(const ImportGuard&) = delete;
    ImportGuard& operator=(const ImportGuard&) = delete;
    ImportGuard(ImportGuard&&) = delete;
    ImportGuard& operator=(ImportGuard&&) = delete;

    /**
     * @brief Check if import is valid (no circular dependency).
     *
     * @return true if import can proceed
     */
    [[nodiscard]] bool isValid() const { return isValid_; }

    /**
     * @brief Check if import would create a cycle.
     *
     * @return true if circular dependency detected
     */
    [[nodiscard]] bool wouldCycle() const { return !isValid_; }

private:
    ModuleCache& cache_;                    ///< Cache being managed
    std::filesystem::path modulePath_;      ///< Module being imported
    bool isValid_;                          ///< Whether import is valid
    bool needsCleanup_;                     ///< Whether endImport needs to be called
};

} // namespace cxy::compiler