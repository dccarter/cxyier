#include "catch2.hpp"
#include <cxy/compiler/compiler.hpp>
#include <cxy/compiler/options.hpp>
#include <cxy/ast/node.hpp>
#include <cxy/diagnostics.hpp>
#include <cxy/memory/arena.hpp>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>

using namespace cxy;
using namespace cxy::compiler;

namespace {
    // Helper to create a temporary file for testing
    class TempFile {
    public:
        explicit TempFile(const std::string& content = "test content", const std::string& extension = ".cxy") {
            path_ = std::filesystem::temp_directory_path() / ("test_" + std::to_string(rand()) + extension);
            std::ofstream file(path_);
            file << content;
            file.close();
        }

        ~TempFile() {
            std::error_code ec;
            std::filesystem::remove(path_, ec);
        }

        const std::filesystem::path& path() const { return path_; }

        void updateContent(const std::string& newContent) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            std::ofstream file(path_);
            file << newContent;
        }

    private:
        std::filesystem::path path_;
    };

    // Helper to create a temporary directory
    class TempDir {
    public:
        TempDir() {
            path_ = std::filesystem::temp_directory_path() / ("test_dir_" + std::to_string(rand()));
            std::filesystem::create_directories(path_);
        }

        ~TempDir() {
            std::error_code ec;
            std::filesystem::remove_all(path_, ec);
        }

        const std::filesystem::path& path() const { return path_; }

        TempFile createFile(const std::string& filename, const std::string& content) {
            auto filePath = path_ / filename;
            std::ofstream file(filePath);
            file << content;
            file.close();
            return TempFile(content, ""); // Empty extension since we're providing full filename
        }

    private:
        std::filesystem::path path_;
    };

    // Create basic compiler options for testing
    CompilerOptions createTestOptions() {
        CompilerOptions options;
        options.setCommand(Command::Build);
        return options;
    }

    // Create simple test source code that matches working parser tests
    const char* SIMPLE_SOURCE = "var x = 42";

    const char* INVALID_SOURCE = R"(
        // Invalid syntax
        var x: = 42 unexpected
    )";
}

TEST_CASE("Compiler constructor and basic properties", "[compiler]") {
    SECTION("constructor with options and default project root") {
        auto options = createTestOptions();
        Compiler compiler(std::move(options));

        REQUIRE(compiler.getOptions().command == Command::Build);
        REQUIRE(compiler.getProjectRoot() == std::filesystem::current_path());
    }

    SECTION("constructor with custom project root") {
        auto options = createTestOptions();
        TempDir tempDir;

        Compiler compiler(std::move(options), tempDir.path());

        REQUIRE(compiler.getProjectRoot() == tempDir.path());
    }

    SECTION("subsystem access methods") {
        auto options = createTestOptions();
        Compiler compiler(std::move(options));

        // Test that we can access all subsystems
        REQUIRE_NOTHROW(compiler.getTypeRegistry());
        REQUIRE_NOTHROW(compiler.getDiagnostics());
        REQUIRE_NOTHROW(compiler.getSourceManager());
        REQUIRE_NOTHROW(compiler.getStringInterner());
        REQUIRE_NOTHROW(compiler.getModuleCache());

        // Test const versions
        const auto& constCompiler = compiler;
        REQUIRE_NOTHROW(constCompiler.getTypeRegistry());
        REQUIRE_NOTHROW(constCompiler.getDiagnostics());
        REQUIRE_NOTHROW(constCompiler.getSourceManager());
        REQUIRE_NOTHROW(constCompiler.getStringInterner());
        REQUIRE_NOTHROW(constCompiler.getModuleCache());
    }
}

TEST_CASE("Compiler string compilation", "[compiler]") {
    auto options = createTestOptions();
    Compiler compiler(std::move(options));

    SECTION("compile simple valid source") {
        auto result = compiler.compileSource(SIMPLE_SOURCE, "test.cxy");

        REQUIRE(result.status == CompilationResult::Status::Success);
        REQUIRE(result.isSuccess());
        REQUIRE_FALSE(result.isFailure());
        REQUIRE(result.ast != nullptr);
        REQUIRE(result.errorCount == 0);
    }

    SECTION("compile with compileString alias") {
        auto result = compiler.compileString(SIMPLE_SOURCE, "test.cxy");

        REQUIRE(result.status == CompilationResult::Status::Success);
        REQUIRE(result.isSuccess());
        REQUIRE(result.ast != nullptr);
    }

    SECTION("compile invalid source") {
        auto result = compiler.compileSource(INVALID_SOURCE, "invalid.cxy");

        REQUIRE(result.status == CompilationResult::Status::ParseError);
        REQUIRE(result.isFailure());
        REQUIRE_FALSE(result.isSuccess());
        REQUIRE(result.errorCount > 0);
    }

    SECTION("empty source code") {
        auto result = compiler.compileSource("", "empty.cxy");

        // Empty source should still parse successfully
        REQUIRE(result.status == CompilationResult::Status::Success);
        REQUIRE(result.isSuccess());
    }

    SECTION("default filename parameter") {
        auto result = compiler.compileSource(SIMPLE_SOURCE);

        REQUIRE(result.status == CompilationResult::Status::Success);
        // Should use default filename "<input>"
    }
}

TEST_CASE("Compiler file compilation", "[compiler]") {
    auto options = createTestOptions();
    Compiler compiler(std::move(options));

    SECTION("compile valid file") {
        TempFile tempFile(SIMPLE_SOURCE);

        auto result = compiler.compileFile(tempFile.path());

        REQUIRE(result.status == CompilationResult::Status::Success);
        REQUIRE(result.isSuccess());
        REQUIRE(result.ast != nullptr);
        REQUIRE(result.errorCount == 0);
    }

    SECTION("compile invalid file") {
        TempFile tempFile(INVALID_SOURCE);

        auto result = compiler.compileFile(tempFile.path());

        REQUIRE(result.status == CompilationResult::Status::ParseError);
        REQUIRE(result.isFailure());
        REQUIRE(result.errorCount > 0);
    }

    SECTION("compile non-existent file") {
        auto nonExistentPath = std::filesystem::path("/non/existent/file.cxy");

        auto result = compiler.compileFile(nonExistentPath);

        REQUIRE(result.status == CompilationResult::Status::IOError);
        REQUIRE(result.isFailure());
        REQUIRE(result.errorCount > 0);
    }

    SECTION("compile file with no read permission") {
        // This test is platform-dependent, skip on Windows or if permissions can't be set
        TempFile tempFile(SIMPLE_SOURCE);

        // Try to remove read permissions
        std::error_code ec;
        std::filesystem::permissions(tempFile.path(),
                                   std::filesystem::perms::none,
                                   std::filesystem::perm_options::replace, ec);

        if (!ec) {
            auto result = compiler.compileFile(tempFile.path());
            REQUIRE(result.status == CompilationResult::Status::IOError);
            REQUIRE(result.isFailure());

            // Restore permissions for cleanup
            std::filesystem::permissions(tempFile.path(),
                                       std::filesystem::perms::owner_all,
                                       std::filesystem::perm_options::replace, ec);
        }
    }
}

TEST_CASE("Compiler module import resolution", "[compiler]") {
    auto options = createTestOptions();
    TempDir projectRoot;
    Compiler compiler(std::move(options), projectRoot.path());

    SECTION("resolve relative import - same directory") {
        // Create a simple module
        auto moduleContent = R"(
            // Simple module
            var moduleVar: i32 = 123
        )";
        auto moduleFile = projectRoot.createFile("module.cxy", moduleContent);

        // Create main file that imports the module
        auto mainContent = R"(
            import "./module.cxy"
            var mainVar: i32 = 456
        )";
        auto mainFile = projectRoot.createFile("main.cxy", mainContent);

        // Test the import resolution indirectly by compiling
        auto result = compiler.compileFile(mainFile.path());

        // Should succeed even if we don't have full semantic analysis yet
        REQUIRE_FALSE(result.isFailure());
    }

    SECTION("resolve relative import - subdirectory") {
        // Create subdirectory structure
        auto subDir = projectRoot.path() / "utils";
        std::filesystem::create_directories(subDir);

        auto moduleContent = R"(
            // Utility module
            var utilVar: i32 = 789
        )";
        std::ofstream moduleFile(subDir / "helper.cxy");
        moduleFile << moduleContent;
        moduleFile.close();

        auto mainContent = R"(
            import "./utils/helper.cxy"
            var mainVar: i32 = 456
        )";
        auto mainFile = projectRoot.createFile("main.cxy", mainContent);

        auto result = compiler.compileFile(mainFile.path());
        REQUIRE_FALSE(result.isFailure());
    }

    SECTION("reject import outside project root") {
        auto mainContent = R"(
            import "../../../etc/passwd"
        )";
        auto mainFile = projectRoot.createFile("main.cxy", mainContent);

        auto result = compiler.compileFile(mainFile.path());

        // Should fail due to security restriction
        REQUIRE(result.isFailure());
        REQUIRE(result.errorCount > 0);
    }
}

TEST_CASE("Compiler module caching", "[compiler]") {
    auto options = createTestOptions();
    TempDir projectRoot;
    Compiler compiler(std::move(options), projectRoot.path());

    SECTION("cache imported module") {
        auto moduleContent = R"(
            // Module to be cached
            var cachedVar: i32 = 999
        )";
        auto moduleFile = projectRoot.createFile("cached.cxy", moduleContent);

        auto mainContent = R"(
            import "./cached.cxy"
        )";
        auto mainFile = projectRoot.createFile("main.cxy", mainContent);

        // First compilation should cache the module
        auto result1 = compiler.compileFile(mainFile.path());
        REQUIRE_FALSE(result1.isFailure());

        auto& moduleCache = compiler.getModuleCache();
        REQUIRE(moduleCache.size() > 0);

        // Second compilation should use cached module
        auto result2 = compiler.compileFile(mainFile.path());
        REQUIRE_FALSE(result2.isFailure());
    }

    SECTION("invalidate cache when module file changes") {
        auto moduleContent = R"(
            var originalVar: i32 = 111
        )";
        auto moduleFile = projectRoot.createFile("changing.cxy", moduleContent);

        auto mainContent = R"(
            import "./changing.cxy"
        )";
        auto mainFile = projectRoot.createFile("main.cxy", mainContent);

        // Initial compilation
        auto result1 = compiler.compileFile(mainFile.path());
        REQUIRE_FALSE(result1.isFailure());

        // Modify the module file
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Ensure timestamp changes
        std::ofstream modFile(moduleFile.path());
        modFile << R"(
            var modifiedVar: i32 = 222
        )";
        modFile.close();

        // Compile again - should detect change and recompile
        auto result2 = compiler.compileFile(mainFile.path());
        REQUIRE_FALSE(result2.isFailure());
    }
}

TEST_CASE("Compiler circular import detection", "[compiler]") {
    auto options = createTestOptions();
    TempDir projectRoot;
    Compiler compiler(std::move(options), projectRoot.path());

    SECTION("detect simple circular import") {
        auto module1Content = R"(
            import "./module2.cxy"
            var var1: i32 = 1
        )";
        auto module1File = projectRoot.createFile("module1.cxy", module1Content);

        auto module2Content = R"(
            import "./module1.cxy"
            var var2: i32 = 2
        )";
        auto module2File = projectRoot.createFile("module2.cxy", module2Content);

        auto result = compiler.compileFile(module1File.path());

        REQUIRE(result.isFailure());
        REQUIRE(result.errorCount > 0);
    }

    SECTION("detect complex circular import chain") {
        auto module1Content = R"(
            import "./module2.cxy"
        )";
        auto module1File = projectRoot.createFile("mod1.cxy", module1Content);

        auto module2Content = R"(
            import "./module3.cxy"
        )";
        auto module2File = projectRoot.createFile("mod2.cxy", module2Content);

        auto module3Content = R"(
            import "./mod1.cxy"
        )";
        auto module3File = projectRoot.createFile("mod3.cxy", module3Content);

        auto result = compiler.compileFile(module1File.path());

        REQUIRE(result.isFailure());
        REQUIRE(result.errorCount > 0);
    }
}

TEST_CASE("Compiler diagnostic integration", "[compiler]") {
    auto options = createTestOptions();
    Compiler compiler(std::move(options));

    SECTION("collect diagnostics from compilation") {
        auto& diagnostics = compiler.getDiagnostics();
        size_t initialCount = diagnostics.getErrorCount() + diagnostics.getWarningCount();

        auto result = compiler.compileSource(INVALID_SOURCE, "test.cxy");

        REQUIRE(result.isFailure());
        size_t finalCount = diagnostics.getErrorCount() + diagnostics.getWarningCount();
        REQUIRE(finalCount > initialCount);
    }

    SECTION("diagnostics cleared between compilations") {
        // This would depend on the implementation - some compilers clear diagnostics,
        // others accumulate them. We'll test the behavior.
        auto result1 = compiler.compileSource(INVALID_SOURCE, "test1.cxy");
        auto count1 = result1.errorCount + result1.warningCount;

        auto result2 = compiler.compileSource(INVALID_SOURCE, "test2.cxy");
        auto count2 = result2.errorCount + result2.warningCount;

        // Each compilation should report its own errors
        REQUIRE(count1 > 0);
        REQUIRE(count2 > 0);
    }
}

TEST_CASE("Compiler error handling edge cases", "[compiler]") {
    auto options = createTestOptions();
    Compiler compiler(std::move(options));

    SECTION("very large source file") {
        // Create a large but valid source file
        std::string largeSource = "// Large file\n";
        for (int i = 0; i < 10000; ++i) {
            largeSource += "var var" + std::to_string(i) + ": i32 = " + std::to_string(i) + "\n";
        }

        auto result = compiler.compileSource(largeSource, "large.cxy");

        // Should handle large files gracefully
        REQUIRE_FALSE(result.isFailure());
    }

    SECTION("deeply nested expressions") {
        // Create deeply nested parentheses
        std::string deepSource = "var x: i32 = ";
        for (int i = 0; i < 100; ++i) {
            deepSource += "(";
        }
        deepSource += "42";
        for (int i = 0; i < 100; ++i) {
            deepSource += ")";
        }

        auto result = compiler.compileSource(deepSource, "deep.cxy");

        // Should handle deep nesting without stack overflow
        REQUIRE_FALSE(result.isFailure());
    }

    SECTION("binary file compilation") {
        // Create a file with binary content
        TempFile binaryFile;
        std::ofstream file(binaryFile.path(), std::ios::binary);
        for (int i = 0; i < 256; ++i) {
            file.put(static_cast<char>(i));
        }
        file.close();

        auto result = compiler.compileFile(binaryFile.path());

        // Should handle binary files gracefully (likely with parse errors)
        REQUIRE(result.isFailure());
        REQUIRE(result.status == CompilationResult::Status::ParseError);
    }
}

TEST_CASE("Compiler library directory resolution", "[compiler]") {
    SECTION("library imports with configured lib directory") {
        auto options = createTestOptions();
        TempDir libDir;
        TempDir projectRoot;

        // Set library directory in options
        options.system.libDir = libDir.path();

        Compiler compiler(std::move(options), projectRoot.path());

        // Create a library module
        auto libModuleContent = R"(
            // Standard library module
            var stdVar: i32 = 42
        )";
        auto libSubDir = libDir.path() / "stdlib";
        std::filesystem::create_directories(libSubDir);
        std::ofstream libFile(libSubDir / "core.cxy");
        libFile << libModuleContent;
        libFile.close();

        // Create main file that imports from library
        auto mainContent = R"(
            import "stdlib/core.cxy";
            let mainVar: int = 123
        )";
        auto mainFile = projectRoot.createFile("main.cxy", mainContent);

        auto result = compiler.compileFile(mainFile.path());

        // Should resolve library import successfully
        REQUIRE_FALSE(result.isFailure());
    }

    SECTION("library import without configured lib directory") {
        auto options = createTestOptions();
        TempDir projectRoot;

        // Don't set library directory
        Compiler compiler(std::move(options), projectRoot.path());

        auto mainContent = R"(
            import "stdlib/nonexistent.cxy";
        )";
        auto mainFile = projectRoot.createFile("main.cxy", mainContent);

        auto result = compiler.compileFile(mainFile.path());

        // Should fail since no library directory is configured
        REQUIRE(result.isFailure());
    }
}
