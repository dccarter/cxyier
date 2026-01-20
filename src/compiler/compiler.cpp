#include <cxy/compiler/compiler.hpp>
#include <cxy/frontend/lexer.hpp>
#include <cxy/frontend/parser.hpp>
#include <cxy/ast/printer.hpp>
#include <cxy/diagnostics.hpp>
#include <cxy/memory/arena.hpp>
#include <fstream>
#include <filesystem>

namespace cxy::compiler {

Compiler::Compiler(CompilerOptions options, std::filesystem::path projectRoot)
    : options_(std::move(options))
    , projectRoot_(std::move(projectRoot))
    , typeRegistry_()
    , diagnostics_()
    , sourceManager_()
    , arena_(1024 * 1024) // 1MB arena
    , stringInterner_(arena_)
    , moduleCache_()
{
    // Initialize the project root as canonical path
    std::error_code ec;
    projectRoot_ = std::filesystem::canonical(projectRoot_, ec);
    if (ec) {
        // If canonicalization fails, use the provided path
        projectRoot_ = std::filesystem::absolute(projectRoot_);
    }
}

Compiler::~Compiler() = default;

CompilationResult Compiler::compileFile(const std::filesystem::path& sourcePath) {
    // Read file content
    std::ifstream file(sourcePath);
    if (!file.is_open()) {
        diagnostics_.error("Cannot open file: " + sourcePath.string(), 
                          Location{sourcePath.string(), Position{1, 1, 0}});
        return createErrorResult(CompilationResult::Status::IOError, 1);
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    if (!file.good() && !file.eof()) {
        diagnostics_.error("Error reading file: " + sourcePath.string(), 
                          Location{sourcePath.string(), Position{1, 1, 0}});
        return createErrorResult(CompilationResult::Status::IOError, 1);
    }
    
    return compileSource(content, sourcePath.string());
}

CompilationResult Compiler::compileSource(std::string_view source, std::string_view filename) {
    return runCompilationPipeline(source, filename, false);
}

CompilationResult Compiler::compileString(std::string_view source, std::string_view filename) {
    return compileSource(source, filename);
}

ast::ASTNode* Compiler::importModule(
    std::string_view modulePath, 
    const std::filesystem::path& currentFile,
    const Location& importLocation) {
    
    // Resolve module path
    auto resolvedPath = resolveModulePath(modulePath, currentFile);
    if (!resolvedPath) {
        diagnostics_.error("Cannot resolve import path: " + std::string(modulePath), importLocation);
        return nullptr;
    }
    
    auto canonicalPath = std::filesystem::canonical(*resolvedPath);
    
    // Check if module is already cached
    auto cachedModule = moduleCache_.getCachedModule(canonicalPath);
    if (cachedModule) {
        // Return the cached AST directly (arena-allocated)
        return const_cast<ast::ASTNode*>(cachedModule);
    }
    
    // Use ImportGuard for cycle detection
    ImportGuard guard(moduleCache_, canonicalPath, diagnostics_);
    if (!guard.isValid()) {
        return nullptr; // Circular dependency detected and reported
    }
    
    // Load and compile the module
    return loadModule(canonicalPath, importLocation);
}

void Compiler::printASTDebug(const ast::ASTNode* ast) const {
    if (!ast) {
        return;
    }
    
    // Get DevOptions to determine printing configuration
    const auto* devOpts = options_.getDevOptions();
    if (!devOpts || !devOpts->printAST) {
        return;
    }
    
    // Configure AST printer based on DevOptions
    ast::PrinterConfig config;
    
    // Set flags based on options
    ast::PrinterFlags flags = ast::PrinterFlags::None;
    
    if (devOpts->withLocation) {
        flags = flags | ast::PrinterFlags::IncludeLocation;
    }
    
    if (!devOpts->withoutAttrs) {
        flags = flags | ast::PrinterFlags::IncludeAttributes;
    }
    
    if (!devOpts->cleanAST) {
        flags = flags | ast::PrinterFlags::IncludeMetadata;
        flags = flags | ast::PrinterFlags::IncludeFlags;
    }
    
    config.flags = flags;
    
    // Create printer with configuration
    ast::ASTPrinter printer(config);
    
    // Determine output destination
    if (options_.outputFile) {
        // Print to output file
        std::ofstream outFile(*options_.outputFile);
        if (outFile.is_open()) {
            outFile << "\n--- AST Output ---" << std::endl;
            printer.print(ast, outFile);
            outFile << std::endl;
            outFile.close();
        } else {
            // Fallback to stdout if file can't be opened
            std::cout << "\n--- AST Output ---" << std::endl;
            printer.print(ast, std::cout);
            std::cout << std::endl;
        }
    } else {
        // Print to stdout
        std::cout << "\n--- AST Output ---" << std::endl;
        printer.print(ast, std::cout);
        std::cout << std::endl;
    }
}

std::optional<std::filesystem::path> Compiler::resolveModulePath(
    std::string_view modulePath,
    const std::filesystem::path& currentFile) const {
    
    if (modulePath.starts_with("./") || modulePath.starts_with("../")) {
        // Relative import: resolve relative to current file's directory
        auto currentDir = currentFile.parent_path();
        auto resolvedPath = currentDir / modulePath;
        
        try {
            resolvedPath = std::filesystem::canonical(resolvedPath);
        } catch (const std::filesystem::filesystem_error&) {
            return std::nullopt; // File doesn't exist
        }
        
        // Security check: ensure resolved path is within project root
        try {
            auto relativePath = std::filesystem::relative(resolvedPath, projectRoot_);
            if (relativePath.empty() || relativePath.string().starts_with("..")) {
                // Path escapes project root - security violation
                return std::nullopt;
            }
        } catch (const std::filesystem::filesystem_error&) {
            return std::nullopt;
        }
        
        return resolvedPath;
    } else {
        // Library import: get library directory from options
        auto libDir = getEffectiveLibDir();
        if (libDir.empty()) {
            return std::nullopt; // No library directory configured
        }
        
        auto libraryPath = libDir / modulePath;
        
        // Add .cxy extension if not present
        if (!libraryPath.has_extension()) {
            libraryPath += ".cxy";
        }
        
        if (std::filesystem::exists(libraryPath)) {
            try {
                return std::filesystem::canonical(libraryPath);
            } catch (const std::filesystem::filesystem_error&) {
                return std::nullopt;
            }
        }
        
        return std::nullopt;
    }
}

std::filesystem::path Compiler::getEffectiveLibDir() const {
    auto libDir = options_.getEffectiveLibDir();
    if (libDir) {
        return *libDir;
    }
    
    // Try to detect from environment variables or default locations
    if (auto env = std::getenv("CXY_STDLIB_DIR")) {
        return std::filesystem::path(env);
    }
    
    if (auto env = std::getenv("CXY_ROOT")) {
        return std::filesystem::path(env) / "lib";
    }
    
    // Default to empty path (no library directory)
    return std::filesystem::path{};
}

ast::ASTNode* Compiler::loadModule(
    const std::filesystem::path& modulePath,
    const Location& importLocation) {
    
    // Read the module file
    std::ifstream file(modulePath);
    if (!file.is_open()) {
        diagnostics_.error("Cannot open imported module: " + modulePath.string(), importLocation);
        return nullptr;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    if (!file.good() && !file.eof()) {
        diagnostics_.error("Error reading imported module: " + modulePath.string(), importLocation);
        return nullptr;
    }
    
    // Compile the module with semantic analysis for complete type information
    auto result = runCompilationPipeline(content, modulePath.string(), true);
    
    if (result.isFailure()) {
        diagnostics_.error("Failed to compile imported module: " + modulePath.string(), importLocation);
        return nullptr;
    }
    
    // Cache the successfully compiled module
    auto ast = result.ast;
    bool cached = moduleCache_.cacheModule(modulePath, 
                                          ast, 
                                          result.errorCount,
                                          result.warningCount,
                                          true); // Has semantic info
    (void)cached; // Suppress unused warning
    
    return ast;
}

CompilationResult Compiler::runCompilationPipeline(
    std::string_view source,
    std::string_view filename,
    bool runSemanticAnalysis) {
    
    // Clear diagnostics for this compilation
    // (Note: this depends on whether we want to accumulate diagnostics across compilations)
    
    // Register source with source manager
    sourceManager_.registerFile(std::string(filename), std::string(source));
    
    try {
        // Lexical Analysis
        Lexer lexer(filename, source, diagnostics_, stringInterner_);
        
        // Token printing debug functionality
        if (auto* devOpts = options_.getDevOptions()) {
            if (devOpts->printTokens) {
                if (options_.outputFile) {
                    // Open output file for token printing
                    std::ofstream outFile(*options_.outputFile);
                    if (outFile.is_open()) {
                        lexer.printAllTokens(outFile);
                        outFile.close();
                    } else {
                        // Fallback to stdout if file can't be opened
                        lexer.printAllTokens(std::cout);
                    }
                } else {
                    // Print to stdout
                    lexer.printAllTokens(std::cout);
                }
                
                // Return immediately after token printing with null AST
                // Check if there were any errors during tokenization
                if (diagnostics_.getErrorCount() > 0) {
                    return createErrorResult(
                        CompilationResult::Status::ParseError, 
                        diagnostics_.getErrorCount(),
                        diagnostics_.getWarningCount()
                    );
                } else {
                    return createSuccessResult(
                        nullptr, // null AST node
                        options_.outputFile ? *options_.outputFile : std::filesystem::path{},
                        diagnostics_.getWarningCount()
                    );
                }
            }
        }
        
        // Parsing
        Parser parser(lexer, arena_, sourceManager_, stringInterner_, diagnostics_, typeRegistry_);
        parser.initialize(); // Initialize token buffer before parsing
        auto ast = parser.parseCompilationUnit();
        
        if (diagnostics_.getErrorCount() > 0) {
            return createErrorResult(CompilationResult::Status::ParseError, 
                                   diagnostics_.getErrorCount(),
                                   diagnostics_.getWarningCount());
        }
        
        if (ast) {
            printASTDebug(ast);
        }
        
        // Semantic Analysis (if requested)
        if (runSemanticAnalysis) {
            // TODO: Implement semantic analysis pass
            // This would include:
            // - Symbol resolution
            // - Type checking
            // - Compile-time evaluation
            // For now, we'll skip this and assume semantic info is available
        }
        
        auto result = createSuccessResult(
            ast, // Raw arena-allocated pointer
            std::filesystem::path{}, // No output file for now
            diagnostics_.getWarningCount());
            
        return result;
        
    } catch (const std::exception& e) {
        diagnostics_.error("Internal compiler error: " + std::string(e.what()), 
                          Location{std::string(filename), Position{1, 1, 0}});
        return createErrorResult(CompilationResult::Status::InternalError, 1);
    }
}

} // namespace cxy::compiler