#include "catch2.hpp"
#include "../../parser_test_utils.hpp"
#include "cxy/ast/declarations.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/types.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::test;

TEST_CASE("Module Declaration Parsing - Basic Forms", "[parser][declarations][module-decl]") {
    SECTION("module utils") {
        auto fixture = createParserFixture("module utils");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->name != nullptr);
        REQUIRE(moduleDecl->topLevel.empty());
        REQUIRE(moduleDecl->mainContent.empty());

        REQUIRE_AST_MATCHES(stmt, R"((ModuleDeclaration
  (Identifier utils)))");
    }

    SECTION("module hello") {
        auto fixture = createParserFixture("module hello");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->name != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((ModuleDeclaration
  (Identifier hello)))");
    }

    SECTION("module math_utils") {
        auto fixture = createParserFixture("module math_utils");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((ModuleDeclaration
  (Identifier math_utils)))");
    }
}

TEST_CASE("Module Declaration Parsing - With Imports", "[parser][declarations][module-decl]") {
    SECTION("module with single import") {
        auto fixture = createParserFixture(R"(module utils
import "core.cxy")");
        auto *stmt = fixture->parseCompilationUnit();

        // Debug: Print diagnostics if parsing failed
        if (stmt == nullptr) {
            std::cout << "Parsing failed for module with single import" << std::endl;
            auto diagnostics = fixture->getDiagnostics();
            for (const auto& diag : diagnostics) {
                std::cout << "Error: " << diag.message << std::endl;
            }
        }

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->topLevel.size() == 1);
        REQUIRE(moduleDecl->mainContent.empty());

        REQUIRE_AST_MATCHES(stmt, R"((ModuleDeclaration
  (Identifier utils)
  (ImportDeclaration
    (String "core.cxy"))))");
    }

    SECTION("module with multiple imports") {
        auto fixture = createParserFixture(R"(module client
import "http.cxy" as http
import "json.cxy" as json)");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->topLevel.size() == 2);
        REQUIRE(moduleDecl->mainContent.empty());

        REQUIRE_AST_MATCHES(stmt, R"((ModuleDeclaration
  (Identifier client)
  (ImportDeclaration
    (String "http.cxy")
    (Identifier http))
  (ImportDeclaration
    (String "json.cxy")
    (Identifier json))))");
    }

    SECTION("module with named imports") {
        auto fixture = createParserFixture(R"(module utils
import dump from "debug.cxy"
import { assert, mock } from "test.cxy")");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->topLevel.size() == 2);
    }
}

TEST_CASE("Module Declaration Parsing - With Main Content", "[parser][declarations][module-decl]") {
    SECTION("module with single function") {
        auto fixture = createParserFixture(R"(module hello
func greet() {
    println("Hello!")
})");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->topLevel.empty());
        REQUIRE(moduleDecl->mainContent.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((ModuleDeclaration
  (Identifier hello)
  (FuncDeclaration
    (Identifier greet)
    (BlockStmt
      (ExprStmt
        (CallExpr
          (Identifier println)
          (String "Hello!")))))))");
    }

    SECTION("module with type declaration") {
        auto fixture = createParserFixture(R"(module types
type UserId = i64)");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->topLevel.empty());
        REQUIRE(moduleDecl->mainContent.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((ModuleDeclaration
  (Identifier types)
  (TypeDeclaration
    (Identifier UserId)
    (Type i64))))");
    }

    SECTION("module with multiple declarations") {
        auto fixture = createParserFixture(R"(module impl
type Result = i32 | string
func process() Result {
    return 42
})");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);


        REQUIRE(moduleDecl->topLevel.empty());
        REQUIRE(moduleDecl->mainContent.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((ModuleDeclaration
  (Identifier impl)
  (TypeDeclaration
    (Identifier Result)
    (UnionType
      (Type i32)
      (Type string)))
  (FuncDeclaration
    (Identifier process)
    (Identifier Result)
    (BlockStmt
      (ReturnStmt
        (Int 42))))))");
    }
}

TEST_CASE("Module Declaration Parsing - Complete Modules", "[parser][declarations][module-decl]") {
    SECTION("module with imports and main content") {
        auto fixture = createParserFixture(R"(module utils
import "core.cxy" as core
import "std/io.cxy" as io

type Result<T> = T | Error
func process(data string) Result<i32> {
    return 42
})");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);

        REQUIRE(moduleDecl->topLevel.size() == 2);
        REQUIRE(moduleDecl->mainContent.size() == 2);

        // Verify imports are in topLevel
        REQUIRE(moduleDecl->topLevel[0]->kind == astImportDeclaration);
        REQUIRE(moduleDecl->topLevel[1]->kind == astImportDeclaration);

        // Verify declarations are in mainContent
        REQUIRE(moduleDecl->mainContent[0]->kind == astGenericDeclaration);
        REQUIRE(moduleDecl->mainContent[1]->kind == astFuncDeclaration);
    }

    SECTION("complex module structure") {
        auto fixture = createParserFixture(R"(module complex
import "external.cxy" as ext
import { helper, util } from "tools.cxy"

type CustomError = string
enum Status {
    Ok,
    Failed
}

func initialize() {
    // setup code
}

func cleanup() {
    // cleanup code
})");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->topLevel.size() == 2);
        REQUIRE(moduleDecl->mainContent.size() == 4);
    }
}

TEST_CASE("Module Declaration Parsing - Error Cases", "[parser][declarations][module-decl]") {
    SECTION("missing module name") {
        auto fixture = createParserFixture("module");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("invalid module name") {
        auto fixture = createParserFixture("module 123invalid");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("module name with special characters") {
        auto fixture = createParserFixture("module my-module");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("incomplete module declaration") {
        auto fixture = createParserFixture("module utils import");
        auto *stmt = fixture->parseCompilationUnit();

        // Should parse module successfully, but import parsing may fail
        REQUIRE(stmt == nullptr);
    }
}

TEST_CASE("Module Declaration Parsing - Edge Cases", "[parser][declarations][module-decl]") {
    SECTION("implicit main module - no module declaration") {
        auto fixture = createParserFixture(R"(func main() {
    println("Hello World!")
})");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->name == nullptr);  // No explicit name for main module
        REQUIRE(moduleDecl->topLevel.empty());
        REQUIRE(moduleDecl->mainContent.size() == 1);
    }

    SECTION("implicit module with imports") {
        auto fixture = createParserFixture(R"(import "std/io.cxy" as io

func main() {
    io.println("Hello!")
})");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->name == nullptr);  // No explicit name
        REQUIRE(moduleDecl->topLevel.size() == 1);
        REQUIRE(moduleDecl->mainContent.size() == 1);
    }

    SECTION("empty named module") {
        auto fixture = createParserFixture("module empty");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->name != nullptr);  // Named module
        REQUIRE(moduleDecl->topLevel.empty());
        REQUIRE(moduleDecl->mainContent.empty());
    }

    SECTION("completely empty file - implicit main module") {
        auto fixture = createParserFixture("");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->name == nullptr);  // Implicit main module
        REQUIRE(moduleDecl->topLevel.empty());
        REQUIRE(moduleDecl->mainContent.empty());
    }

    SECTION("module with only imports") {
        auto fixture = createParserFixture(R"(module imports_only
import "lib1.cxy"
import "lib2.cxy")");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->topLevel.size() == 2);
        REQUIRE(moduleDecl->mainContent.empty());
    }

    SECTION("module with only main content") {
        auto fixture = createParserFixture(R"(module content_only
func helper() i32 { return 42 })");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->topLevel.empty());
        REQUIRE(moduleDecl->mainContent.size() == 1);
    }

    SECTION("module with underscore in name") {
        auto fixture = createParserFixture("module my_valid_module");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((ModuleDeclaration
  (Identifier my_valid_module)))");
    }
}

TEST_CASE("Module Declaration Parsing - Whitespace Handling", "[parser][declarations][module-decl]") {
    SECTION("module with newlines between sections") {
        auto fixture = createParserFixture(R"(module spaced
import "core.cxy"

func check() {
    // test
})");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->topLevel.size() == 1);
        REQUIRE(moduleDecl->mainContent.size() == 1);
    }

    SECTION("module with compact formatting") {
        auto fixture = createParserFixture(R"(module compact
import "lib.cxy"
func check(){})");
        auto *stmt = fixture->parseCompilationUnit();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astModuleDeclaration);

        auto *moduleDecl = static_cast<ModuleDeclarationNode *>(stmt);
        REQUIRE(moduleDecl->topLevel.size() == 1);
        REQUIRE(moduleDecl->mainContent.size() == 1);
    }
}
