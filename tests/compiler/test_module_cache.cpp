#include "catch2.hpp"
#include <cxy/compiler/module_cache.hpp>
#include <cxy/ast/node.hpp>
#include <cxy/diagnostics.hpp>
#include <cxy/memory/arena.hpp>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace cxy;
using namespace cxy::compiler;

namespace {
    // Helper to create a temporary file for testing
    class TempFile {
    public:
        explicit TempFile(const std::string& content = "test content") {
            path_ = std::filesystem::temp_directory_path() / ("test_" + std::to_string(rand()) + ".cxy");
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
            // Small delay to ensure timestamp changes
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            std::ofstream file(path_);
            file << newContent;
        }
        
    private:
        std::filesystem::path path_;
    };

    // Helper to create AST nodes for testing
    ast::ASTNode* createTestAST(ArenaAllocator& arena, const std::string& name = "test") {
        auto* ast = new(arena.allocate(sizeof(ast::ASTNode), alignof(ast::ASTNode))) 
                       ast::ASTNode(ast::NodeKind::astNoop, 
                                    Location{name, Position{1, 1, 0}}, 
                                    arena);
        return ast;
    }
}

TEST_CASE("CachedModule struct", "[module_cache]") {
    ArenaAllocator arena;
    
    SECTION("default initialization") {
        CachedModule module;
        REQUIRE(module.ast == nullptr);
        REQUIRE(module.errorCount == 0);
        REQUIRE(module.warningCount == 0);
        REQUIRE(module.hasSemanticInfo == false);
        REQUIRE_FALSE(module.isSuccessful());
    }
    
    SECTION("successful module") {
        CachedModule module;
        module.ast = createTestAST(arena);
        module.errorCount = 0;
        module.warningCount = 2;
        module.hasSemanticInfo = true;
        
        REQUIRE(module.isSuccessful());
    }
    
    SECTION("module with errors") {
        CachedModule module;
        module.ast = createTestAST(arena);
        module.errorCount = 1;
        module.warningCount = 0;
        
        REQUIRE_FALSE(module.isSuccessful());
    }
    
    SECTION("module without AST") {
        CachedModule module;
        module.ast = nullptr;
        module.errorCount = 0;
        
        REQUIRE_FALSE(module.isSuccessful());
    }
}

TEST_CASE("ModuleCache basic operations", "[module_cache]") {
    ModuleCache cache;
    ArenaAllocator arena;
    
    SECTION("empty cache") {
        REQUIRE(cache.empty());
        REQUIRE(cache.size() == 0);
        
        auto path = std::filesystem::path("/nonexistent/module.cxy");
        REQUIRE_FALSE(cache.isCached(path));
        REQUIRE(cache.getCachedModule(path) == nullptr);
        REQUIRE(cache.getModuleInfo(path) == nullptr);
    }
    
    SECTION("cache module") {
        TempFile tempFile;
        auto ast = createTestAST(arena, "test_module");
        auto astPtr = ast;
        
        bool result = cache.cacheModule(tempFile.path(), ast, 0, 1, true);
        
        REQUIRE(result);
        REQUIRE_FALSE(cache.empty());
        REQUIRE(cache.size() == 1);
        REQUIRE(cache.isCached(tempFile.path()));
        
        auto cachedAST = cache.getCachedModule(tempFile.path());
        REQUIRE(cachedAST == astPtr);
        
        auto moduleInfo = cache.getModuleInfo(tempFile.path());
        REQUIRE(moduleInfo != nullptr);
        REQUIRE(moduleInfo->errorCount == 0);
        REQUIRE(moduleInfo->warningCount == 1);
        REQUIRE(moduleInfo->hasSemanticInfo == true);
        REQUIRE(moduleInfo->isSuccessful());
    }
    
    SECTION("cache multiple modules") {
        TempFile file1("module1");
        TempFile file2("module2");
        
        auto ast1 = createTestAST(arena, "module1");
        auto ast2 = createTestAST(arena, "module2");
        
        REQUIRE(cache.cacheModule(file1.path(), ast1));
        REQUIRE(cache.cacheModule(file2.path(), ast2));
        
        REQUIRE(cache.size() == 2);
        REQUIRE(cache.isCached(file1.path()));
        REQUIRE(cache.isCached(file2.path()));
    }
    
    SECTION("remove module") {
        TempFile tempFile;
        auto ast = createTestAST(arena);
        
        REQUIRE(cache.cacheModule(tempFile.path(), ast));
        REQUIRE(cache.isCached(tempFile.path()));
        
        bool removed = cache.removeModule(tempFile.path());
        REQUIRE(removed);
        REQUIRE_FALSE(cache.isCached(tempFile.path()));
        REQUIRE(cache.size() == 0);
        
        // Try to remove non-existent module
        bool removedAgain = cache.removeModule(tempFile.path());
        REQUIRE_FALSE(removedAgain);
    }
    
    SECTION("clear cache") {
        TempFile file1;
        TempFile file2;
        
        REQUIRE(cache.cacheModule(file1.path(), createTestAST(arena)));
        REQUIRE(cache.cacheModule(file2.path(), createTestAST(arena)));
        REQUIRE(cache.size() == 2);
        
        cache.clear();
        REQUIRE(cache.empty());
        REQUIRE(cache.size() == 0);
    }
}

TEST_CASE("ModuleCache import cycle detection", "[module_cache]") {
    ModuleCache cache;
    
    auto path1 = std::filesystem::path("/test/module1.cxy");
    auto path2 = std::filesystem::path("/test/module2.cxy");
    auto path3 = std::filesystem::path("/test/module3.cxy");
    
    SECTION("no cycle - single import") {
        REQUIRE_FALSE(cache.wouldCreateCycle(path1));
        REQUIRE(cache.beginImport(path1));
        
        auto stack = cache.getImportStack();
        REQUIRE(stack.size() == 1);
        REQUIRE(stack[0] == path1);
        
        cache.endImport(path1);
        REQUIRE(cache.getImportStack().empty());
    }
    
    SECTION("no cycle - nested imports") {
        REQUIRE(cache.beginImport(path1));
        REQUIRE_FALSE(cache.wouldCreateCycle(path2));
        REQUIRE(cache.beginImport(path2));
        
        auto stack = cache.getImportStack();
        REQUIRE(stack.size() == 2);
        REQUIRE(stack[0] == path1);
        REQUIRE(stack[1] == path2);
        
        cache.endImport(path2);
        cache.endImport(path1);
        REQUIRE(cache.getImportStack().empty());
    }
    
    SECTION("direct cycle detection") {
        REQUIRE(cache.beginImport(path1));
        REQUIRE(cache.wouldCreateCycle(path1));
        REQUIRE_FALSE(cache.beginImport(path1)); // Should fail
        
        cache.endImport(path1);
    }
    
    SECTION("indirect cycle detection") {
        REQUIRE(cache.beginImport(path1));
        REQUIRE(cache.beginImport(path2));
        REQUIRE(cache.beginImport(path3));
        
        // Try to import path1 again - should create cycle
        REQUIRE(cache.wouldCreateCycle(path1));
        REQUIRE_FALSE(cache.beginImport(path1));
        
        cache.endImport(path3);
        cache.endImport(path2);
        cache.endImport(path1);
    }
    
    SECTION("import stack order") {
        REQUIRE(cache.beginImport(path1));
        REQUIRE(cache.beginImport(path2));
        REQUIRE(cache.beginImport(path3));
        
        auto stack = cache.getImportStack();
        REQUIRE(stack.size() == 3);
        REQUIRE(stack[0] == path1);
        REQUIRE(stack[1] == path2);
        REQUIRE(stack[2] == path3);
        
        cache.endImport(path3);
        cache.endImport(path2);
        cache.endImport(path1);
    }
}

TEST_CASE("ModuleCache file modification tracking", "[module_cache]") {
    ModuleCache cache;
    ArenaAllocator arena;
    
    SECTION("invalidate modified file") {
        TempFile tempFile("original content");
        auto ast = createTestAST(arena);
        
        // Cache the module
        REQUIRE(cache.cacheModule(tempFile.path(), ast));
        REQUIRE(cache.isCached(tempFile.path()));
        
        // Modify the file
        tempFile.updateContent("modified content");
        
        // Check if invalidation works
        bool invalidated = cache.invalidateIfModified(tempFile.path());
        REQUIRE(invalidated);
        REQUIRE_FALSE(cache.isCached(tempFile.path()));
    }
    
    SECTION("don't invalidate unmodified file") {
        TempFile tempFile;
        auto ast = createTestAST(arena);
        
        REQUIRE(cache.cacheModule(tempFile.path(), ast));
        REQUIRE(cache.isCached(tempFile.path()));
        
        // Don't modify the file, just check
        bool invalidated = cache.invalidateIfModified(tempFile.path());
        REQUIRE_FALSE(invalidated);
        REQUIRE(cache.isCached(tempFile.path()));
    }
    
    SECTION("invalidate all modified files") {
        TempFile file1("content1");
        TempFile file2("content2");
        TempFile file3("content3");
        
        // Cache all files
        REQUIRE(cache.cacheModule(file1.path(), createTestAST(arena)));
        REQUIRE(cache.cacheModule(file2.path(), createTestAST(arena)));
        REQUIRE(cache.cacheModule(file3.path(), createTestAST(arena)));
        REQUIRE(cache.size() == 3);
        
        // Modify only file1 and file3
        file1.updateContent("modified1");
        file3.updateContent("modified3");
        
        size_t invalidated = cache.invalidateModified();
        REQUIRE(invalidated == 2);
        REQUIRE(cache.size() == 1);
        REQUIRE(cache.isCached(file2.path())); // Only file2 should remain
    }
    
    SECTION("handle non-existent file") {
        auto nonExistentPath = std::filesystem::path("/non/existent/file.cxy");
        ArenaAllocator arena;
        
        // This should not crash, but return false
        bool invalidated = cache.invalidateIfModified(nonExistentPath);
        REQUIRE_FALSE(invalidated);
    }
}

TEST_CASE("ModuleCache semantic info tracking", "[module_cache]") {
    ModuleCache cache;
    ArenaAllocator arena;
    
    SECTION("track semantic info status") {
        TempFile file1;
        TempFile file2;
        
        // Cache one module with semantic info, one without
        REQUIRE(cache.cacheModule(file1.path(), createTestAST(arena), 0, 0, true));
        REQUIRE(cache.cacheModule(file2.path(), createTestAST(arena), 0, 0, false));
        
        REQUIRE_FALSE(cache.allModulesHaveSemanticInfo());
        
        // Remove the module without semantic info
        REQUIRE(cache.removeModule(file2.path()));
        REQUIRE(cache.allModulesHaveSemanticInfo());
        
        // Add another module with semantic info
        TempFile file3;
        REQUIRE(cache.cacheModule(file3.path(), createTestAST(arena), 0, 0, true));
        REQUIRE(cache.allModulesHaveSemanticInfo());
    }
    
    SECTION("empty cache has semantic info") {
        // Empty cache should return true (vacuous truth)
        REQUIRE(cache.allModulesHaveSemanticInfo());
    }
}

TEST_CASE("ImportGuard RAII behavior", "[module_cache]") {
    ModuleCache cache;
    DiagnosticLogger diagnostics;
    
    auto path1 = std::filesystem::path("/test/module1.cxy");
    auto path2 = std::filesystem::path("/test/module2.cxy");
    
    SECTION("successful import guard") {
        REQUIRE(cache.getImportStack().empty());
        
        {
            ImportGuard guard(cache, path1, diagnostics);
            REQUIRE(guard.isValid());
            REQUIRE_FALSE(guard.wouldCycle());
            REQUIRE(cache.getImportStack().size() == 1);
        }
        
        // Should be cleaned up automatically
        REQUIRE(cache.getImportStack().empty());
    }
    
    SECTION("nested import guards") {
        {
            ImportGuard guard1(cache, path1, diagnostics);
            REQUIRE(guard1.isValid());
            
            {
                ImportGuard guard2(cache, path2, diagnostics);
                REQUIRE(guard2.isValid());
                REQUIRE(cache.getImportStack().size() == 2);
            }
            
            REQUIRE(cache.getImportStack().size() == 1);
        }
        
        REQUIRE(cache.getImportStack().empty());
    }
    
    SECTION("circular dependency detection") {
        REQUIRE(cache.beginImport(path1)); // Manually add to create cycle condition
        
        {
            ImportGuard guard(cache, path1, diagnostics);
            REQUIRE_FALSE(guard.isValid());
            REQUIRE(guard.wouldCycle());
        }
        
        cache.endImport(path1); // Clean up manual import
    }
    
    SECTION("exception safety") {
        auto testException = []() {
            throw std::runtime_error("test exception");
        };
        
        try {
            ImportGuard guard(cache, path1, diagnostics);
            REQUIRE(cache.getImportStack().size() == 1);
            testException();
        } catch (const std::runtime_error&) {
            // Exception occurred, but guard should still clean up
        }
        
        REQUIRE(cache.getImportStack().empty());
    }
}

TEST_CASE("ModuleCache edge cases", "[module_cache]") {
    ModuleCache cache;
    ArenaAllocator arena;
    
    SECTION("cache nullptr AST") {
        TempFile tempFile;
        
        bool result = cache.cacheModule(tempFile.path(), nullptr);
        REQUIRE(result); // Should succeed but store nullptr
        REQUIRE(cache.isCached(tempFile.path()));
        
        auto moduleInfo = cache.getModuleInfo(tempFile.path());
        REQUIRE(moduleInfo != nullptr);
        REQUIRE_FALSE(moduleInfo->isSuccessful()); // nullptr AST = not successful
    }
    
    SECTION("cache same path twice") {
        TempFile tempFile;
        auto ast1 = createTestAST(arena, "first");
        auto ast2 = createTestAST(arena, "second");
        auto ast2Ptr = ast2;
        
        REQUIRE(cache.cacheModule(tempFile.path(), ast1));
        REQUIRE(cache.cacheModule(tempFile.path(), ast2)); // Should overwrite
        
        REQUIRE(cache.size() == 1);
        auto cachedAST = cache.getCachedModule(tempFile.path());
        REQUIRE(cachedAST == ast2Ptr); // Should be the second AST
    }
    
    SECTION("move semantics") {
        ModuleCache cache1;
        TempFile tempFile;
        
        REQUIRE(cache1.cacheModule(tempFile.path(), createTestAST(arena)));
        REQUIRE(cache1.size() == 1);
        
        ModuleCache cache2 = std::move(cache1);
        REQUIRE(cache2.size() == 1);
        REQUIRE(cache2.isCached(tempFile.path()));
        
        // Original cache should be in moved-from state
        REQUIRE(cache1.size() == 0);
    }
}