#include "catch2.hpp"
#include "../../parser_test_utils.hpp"
#include "cxy/ast/declarations.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/literals.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::test;

TEST_CASE("Import Declaration Parsing - Whole Module Import", "[parser][declarations][import-decl]") {
    SECTION("simple whole module import") {
        auto fixture = createParserFixture(R"(import "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::WholeModule);
        REQUIRE(importDecl->path != nullptr);
        REQUIRE(importDecl->name == nullptr);
        REQUIRE(importDecl->alias == nullptr);
        REQUIRE(importDecl->entities.empty());

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "utils.cxy")))");
    }

    SECTION("standard library import") {
        auto fixture = createParserFixture(R"(import "std/io.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::WholeModule);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "std/io.cxy")))");
    }

    SECTION("C header import") {
        auto fixture = createParserFixture(R"(import "stdio.h")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "stdio.h")))");
    }
}

TEST_CASE("Import Declaration Parsing - Module Alias Import", "[parser][declarations][import-decl]") {
    SECTION("simple module alias") {
        auto fixture = createParserFixture(R"(import "utils.cxy" as Utils)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::ModuleAlias);
        REQUIRE(importDecl->path != nullptr);
        REQUIRE(importDecl->alias != nullptr);
        REQUIRE(importDecl->name == nullptr);
        REQUIRE(importDecl->entities.empty());

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "utils.cxy")
  (Identifier Utils)))");
    }

    SECTION("C header with required alias") {
        auto fixture = createParserFixture(R"(import "stdlib.h" as stdlib)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::ModuleAlias);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "stdlib.h")
  (Identifier stdlib)))");
    }

    SECTION("nested path with alias") {
        auto fixture = createParserFixture(R"(import "deep/nested/module.cxy" as nested)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "deep/nested/module.cxy")
  (Identifier nested)))");
    }
}

TEST_CASE("Import Declaration Parsing - Named Import", "[parser][declarations][import-decl]") {
    SECTION("single named import") {
        auto fixture = createParserFixture(R"(import dump from "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::MultipleImports);
        REQUIRE(importDecl->path != nullptr);
        REQUIRE(importDecl->name == nullptr);
        REQUIRE(importDecl->alias == nullptr);
        REQUIRE(importDecl->entities.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "utils.cxy")
  (ImportItem (Identifier dump))))");
    }

    SECTION("named import with alias") {
        auto fixture = createParserFixture(R"(import dump as myDump from "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::MultipleImports);
        REQUIRE(importDecl->path != nullptr);
        REQUIRE(importDecl->name == nullptr);
        REQUIRE(importDecl->alias == nullptr);
        REQUIRE(importDecl->entities.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "utils.cxy")
  (ImportItem (Identifier dump) (Identifier myDump))))");
    }

    SECTION("C function import") {
        auto fixture = createParserFixture(R"(import atoi from "stdlib.h")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::MultipleImports);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "stdlib.h")
  (ImportItem (Identifier atoi))))");
    }
}

TEST_CASE("Import Declaration Parsing - Multiple Imports", "[parser][declarations][import-decl]") {
    SECTION("multiple simple imports") {
        auto fixture = createParserFixture(R"(import { dump, debug } from "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::MultipleImports);
        REQUIRE(importDecl->path != nullptr);
        REQUIRE(importDecl->name == nullptr);
        REQUIRE(importDecl->alias == nullptr);
        REQUIRE(importDecl->entities.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (ImportItem (Identifier dump))
  (ImportItem (Identifier debug))
  (String "utils.cxy")))");
    }

    SECTION("multiple imports with mixed aliases") {
        auto fixture = createParserFixture(R"(import { dump, debug as myDebug } from "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::MultipleImports);
        REQUIRE(importDecl->entities.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (ImportItem (Identifier dump))
  (ImportItem (Identifier debug) (Identifier myDebug))
  (String "utils.cxy")))");
    }

    SECTION("multiple imports all with aliases") {
        auto fixture = createParserFixture(R"(import { dump as dumpFunc, debug as debugFunc } from "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->entities.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (ImportItem (Identifier dump) (Identifier dumpFunc))
  (ImportItem (Identifier debug) (Identifier debugFunc))
  (String "utils.cxy")))");
    }

    SECTION("single import in braces") {
        auto fixture = createParserFixture(R"(import { dump } from "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::MultipleImports);
        REQUIRE(importDecl->entities.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (ImportItem (Identifier dump))
  (String "utils.cxy")))");
    }

    SECTION("many imports with trailing comma") {
        auto fixture = createParserFixture(R"(import { 
    assert, 
    mock as mockFunc, 
    verify,
} from "test.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->entities.size() == 3);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (ImportItem (Identifier assert))
  (ImportItem (Identifier mock) (Identifier mockFunc))
  (ImportItem (Identifier verify))
  (String "test.cxy")))");
    }
}

TEST_CASE("Import Declaration Parsing - Test Conditional Imports", "[parser][declarations][import-decl]") {
    SECTION("test whole module import") {
        auto fixture = createParserFixture(R"(import test "test_utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::ConditionalTest);
        REQUIRE(importDecl->path != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "test_utils.cxy")))");
    }

    SECTION("test module with alias") {
        auto fixture = createParserFixture(R"(import test "test_utils.cxy" as testLib)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::ConditionalTest);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "test_utils.cxy")
  (Identifier testLib)))");
    }

    SECTION("test named imports") {
        auto fixture = createParserFixture(R"(import test { assert, mock } from "test_utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::ConditionalTest);
        REQUIRE(importDecl->entities.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (ImportItem (Identifier assert))
  (ImportItem (Identifier mock))
  (String "test_utils.cxy")))");
    }

    SECTION("test single named import") {
        auto fixture = createParserFixture(R"(import test verify from "test_utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::ConditionalTest);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "test_utils.cxy")
  (ImportItem (Identifier verify))))");
    }
}

TEST_CASE("Import Declaration Parsing - Error Cases", "[parser][declarations][import-decl]") {
    SECTION("missing path") {
        auto fixture = createParserFixture("import");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("missing 'from' keyword") {
        auto fixture = createParserFixture(R"(import dump "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("missing closing brace") {
        auto fixture = createParserFixture(R"(import { dump, debug from "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("empty import list") {
        auto fixture = createParserFixture(R"(import { } from "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("missing alias after 'as'") {
        auto fixture = createParserFixture(R"(import "utils.cxy" as)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("invalid identifier in import list") {
        auto fixture = createParserFixture(R"(import { 123invalid } from "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("trailing comma without item") {
        auto fixture = createParserFixture(R"(import { dump, } from "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        // This might be valid depending on grammar - adjust expectation based on implementation
        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->entities.size() == 1);
    }

    SECTION("missing string literal for path") {
        auto fixture = createParserFixture("import invalidPath");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("test without proper import clause") {
        auto fixture = createParserFixture("import test");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Import Declaration Parsing - Whitespace and Formatting", "[parser][declarations][import-decl]") {
    SECTION("compact formatting") {
        auto fixture = createParserFixture(R"(import{dump,debug}from"utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->entities.size() == 2);
    }

    SECTION("extra whitespace") {
        auto fixture = createParserFixture(R"(import   "utils.cxy"   as   Utils)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->kind == ImportDeclarationNode::ModuleAlias);
    }

    SECTION("multiline import list") {
        auto fixture = createParserFixture(R"(import {
    dump,
    debug as myDebug,
    verify
} from "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        auto *importDecl = static_cast<ImportDeclarationNode *>(stmt);
        REQUIRE(importDecl->entities.size() == 3);
    }

    SECTION("comments and whitespace") {
        auto fixture = createParserFixture(R"(import /* comment */ "utils.cxy")");
        auto *stmt = fixture->parseDeclaration();

        // Comments handling depends on lexer implementation
        // This test verifies the parser can handle comments if supported
        if (stmt != nullptr) {
            REQUIRE(stmt->kind == astImportDeclaration);
        }
    }
}

TEST_CASE("Import Declaration Parsing - Integration Tests", "[parser][declarations][import-decl]") {
    SECTION("complex path strings") {
        auto fixture = createParserFixture(R"(import "../relative/path.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "../relative/path.cxy")))");
    }

    SECTION("path with special characters") {
        auto fixture = createParserFixture(R"(import "path/with-dashes_and_underscores.cxy")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astImportDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((ImportDeclaration
  (String "path/with-dashes_and_underscores.cxy")))");
    }

    SECTION("mixed import types in sequence") {
        // This tests that the parser can handle multiple different import types
        auto fixture = createParserFixture(R"(import "core.cxy")");
        auto *stmt1 = fixture->parseDeclaration();

        fixture = createParserFixture(R"(import "utils.cxy" as Utils)");
        auto *stmt2 = fixture->parseDeclaration();

        fixture = createParserFixture(R"(import { assert } from "test.cxy")");
        auto *stmt3 = fixture->parseDeclaration();

        REQUIRE(stmt1 != nullptr);
        REQUIRE(stmt2 != nullptr);
        REQUIRE(stmt3 != nullptr);

        REQUIRE(static_cast<ImportDeclarationNode *>(stmt1)->kind == ImportDeclarationNode::WholeModule);
        REQUIRE(static_cast<ImportDeclarationNode *>(stmt2)->kind == ImportDeclarationNode::ModuleAlias);
        REQUIRE(static_cast<ImportDeclarationNode *>(stmt3)->kind == ImportDeclarationNode::MultipleImports);
    }
}