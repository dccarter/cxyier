#pragma once

#include <cxy/ast/node.hpp>
#include <filesystem>
#include <memory>

namespace cxy::compiler {

/**
 * @brief Result of a compilation operation.
 *
 * This structure contains the outcome of compiling source code,
 * including success/failure status, generated AST, output paths,
 * and diagnostic counts.
 */
struct CompilationResult {
    /**
     * @brief Status codes for compilation operations.
     */
    enum class Status {
        Success,         ///< Compilation completed successfully
        ParseError,      ///< Syntax errors in source code
        SemanticError,   ///< Type checking or semantic analysis errors
        IOError,         ///< File system or I/O related errors
        InternalError    ///< Compiler internal errors
    };

    Status status = Status::Success;                    ///< Overall compilation status
    ast::ASTNode* ast = nullptr;                        ///< Generated AST (arena-allocated, if successful)
    std::filesystem::path outputPath;                   ///< Path to generated output file
    size_t errorCount = 0;                              ///< Number of errors encountered
    size_t warningCount = 0;                            ///< Number of warnings encountered

    /**
     * @brief Default constructor.
     */
    CompilationResult() = default;

    /**
     * @brief Move constructor.
     */
    CompilationResult(CompilationResult&&) = default;

    /**
     * @brief Move assignment operator.
     */
    CompilationResult& operator=(CompilationResult&&) = default;

    /**
     * @brief Copy constructor is deleted (AST is arena-allocated).
     */
    CompilationResult(const CompilationResult&) = delete;

    /**
     * @brief Copy assignment is deleted (AST is arena-allocated).
     */
    CompilationResult& operator=(const CompilationResult&) = delete;

    /**
     * @brief Check if compilation was successful.
     * @return true if status is Success and no errors occurred
     */
    [[nodiscard]] bool isSuccess() const {
        return status == Status::Success && errorCount == 0;
    }

    /**
     * @brief Check if compilation failed.
     * @return true if status is not Success or errors occurred
     */
    [[nodiscard]] bool isFailure() const {
        return !isSuccess();
    }

    /**
     * @brief Check if compilation produced warnings.
     * @return true if warning count is greater than zero
     */
    [[nodiscard]] bool hasWarnings() const {
        return warningCount > 0;
    }

    /**
     * @brief Check if compilation produced errors.
     * @return true if error count is greater than zero
     */
    [[nodiscard]] bool hasErrors() const {
        return errorCount > 0;
    }

    /**
     * @brief Get a human-readable status string.
     * @return String representation of the compilation status
     */
    [[nodiscard]] std::string getStatusString() const {
        switch (status) {
            case Status::Success:
                return "Success";
            case Status::ParseError:
                return "Parse Error";
            case Status::SemanticError:
                return "Semantic Error";
            case Status::IOError:
                return "I/O Error";
            case Status::InternalError:
                return "Internal Error";
        }
        return "Unknown";
    }
};

/**
 * @brief Create a successful compilation result.
 * @param ast Generated AST
 * @param outputPath Path to output file
 * @param warningCount Number of warnings
 * @return CompilationResult with Success status
 */
[[nodiscard]] inline CompilationResult createSuccessResult(
    ast::ASTNode* ast,
    std::filesystem::path outputPath = {},
    size_t warningCount = 0
) {
    CompilationResult result;
    result.status = CompilationResult::Status::Success;
    result.ast = ast;
    result.outputPath = std::move(outputPath);
    result.warningCount = warningCount;
    return result;
}

/**
 * @brief Create a failed compilation result.
 * @param status Error status
 * @param errorCount Number of errors
 * @param warningCount Number of warnings
 * @return CompilationResult with specified error status
 */
[[nodiscard]] inline CompilationResult createErrorResult(
    CompilationResult::Status status,
    size_t errorCount = 1,
    size_t warningCount = 0
) {
    CompilationResult result;
    result.status = status;
    result.errorCount = errorCount;
    result.warningCount = warningCount;
    return result;
}

} // namespace cxy::compiler