#include "catch2.hpp"
#include "../../parser_test_utils.hpp"
#include "cxy/ast/declarations.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/types.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::test;

TEST_CASE("Variable Declaration Parsing - Basic Forms", "[parser][statements][var-decl]") {
    SECTION("var x = 42") {
        auto fixture = createParserFixture("var x = 42");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *varDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(!varDecl->isConst());
        REQUIRE(varDecl->names.size() == 1);
        REQUIRE(varDecl->type == nullptr);
        REQUIRE(varDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier x)
  (Int 42)))");
    }

    SECTION("const PI = 3.14") {
        auto fixture = createParserFixture("const PI = 3.14");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *constDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(constDecl->isConst());
        REQUIRE(constDecl->names.size() == 1);
        REQUIRE(constDecl->type == nullptr);
        REQUIRE(constDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier PI)
  (Float 3.14)))");
    }

    SECTION("auto name = \"John\"") {
        auto fixture = createParserFixture("auto name = \"John\"");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *autoDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(!autoDecl->isConst());
        REQUIRE(autoDecl->names.size() == 1);
        REQUIRE(autoDecl->type == nullptr);
        REQUIRE(autoDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier name)
  (String "John")))");
    }
}

TEST_CASE("Variable Declaration Parsing - Type Annotations", "[parser][statements][var-decl][types]") {
    SECTION("var count: i32 = 0") {
        auto fixture = createParserFixture("var count: i32 = 0");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *varDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(!varDecl->isConst());
        REQUIRE(varDecl->names.size() == 1);
        REQUIRE(varDecl->type != nullptr);
        REQUIRE(varDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier count)
  (Type i32)
  (Int 0)))");
    }

    SECTION("const user: string") {
        auto fixture = createParserFixture("const user: string");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *constDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(constDecl->isConst());
        REQUIRE(constDecl->names.size() == 1);
        REQUIRE(constDecl->type != nullptr);
        REQUIRE(constDecl->initializer == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier user)
  (Type string)))");
    }

    SECTION("auto value: f64 = 100.0") {
        auto fixture = createParserFixture("auto value: f64 = 100.0");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *autoDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(!autoDecl->isConst());
        REQUIRE(autoDecl->names.size() == 1);
        REQUIRE(autoDecl->type != nullptr);
        REQUIRE(autoDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier value)
  (Type f64)
  (Float 100)))");
    }
}

TEST_CASE("Variable Declaration Parsing - Multiple Names", "[parser][statements][var-decl][multiple]") {
    SECTION("var a, b = 10") {
        auto fixture = createParserFixture("var a, b = 10");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *varDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(!varDecl->isConst());
        REQUIRE(varDecl->names.size() == 2);
        REQUIRE(varDecl->type == nullptr);
        REQUIRE(varDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier a)
  (Identifier b)
  (Int 10)))");
    }

    SECTION("var x, y, z = getTuple()") {
        auto fixture = createParserFixture("var x, y, z = getTuple()");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *varDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(!varDecl->isConst());
        REQUIRE(varDecl->names.size() == 3);
        REQUIRE(varDecl->type == nullptr);
        REQUIRE(varDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier x)
  (Identifier y)
  (Identifier z)
  (CallExpr
    (Identifier getTuple))))");
    }

    SECTION("const name, age: i32 = getInfo()") {
        auto fixture = createParserFixture("const name, age: i32 = getInfo()");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *constDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(constDecl->isConst());
        REQUIRE(constDecl->names.size() == 2);
        REQUIRE(constDecl->type != nullptr);
        REQUIRE(constDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier name)
  (Identifier age)
  (Type i32)
  (CallExpr
    (Identifier getInfo))))");
    }

    SECTION("var a, b, c: bool") {
        auto fixture = createParserFixture("var a, b, c: bool");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *varDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(!varDecl->isConst());
        REQUIRE(varDecl->names.size() == 3);
        REQUIRE(varDecl->type != nullptr);
        REQUIRE(varDecl->initializer == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier a)
  (Identifier b)
  (Identifier c)
  (Type bool)))");
    }
}

TEST_CASE("Variable Declaration Parsing - Trailing Commas", "[parser][statements][var-decl][trailing-comma]") {
    SECTION("var first, second, = getLargerTuple()") {
        auto fixture = createParserFixture("var first, second, = getLargerTuple()");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *varDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(!varDecl->isConst());
        REQUIRE(varDecl->names.size() == 2);
        REQUIRE(varDecl->type == nullptr);
        REQUIRE(varDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier first)
  (Identifier second)
  (CallExpr
    (Identifier getLargerTuple))))");
    }

    SECTION("const a, b, : i32 = getValues()") {
        auto fixture = createParserFixture("const a, b, : i32 = getValues()");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *constDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(constDecl->isConst());
        REQUIRE(constDecl->names.size() == 2);
        REQUIRE(constDecl->type != nullptr);
        REQUIRE(constDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier a)
  (Identifier b)
  (Type i32)
  (CallExpr
    (Identifier getValues))))");
    }
}

TEST_CASE("Variable Declaration Parsing - Discard Patterns", "[parser][statements][var-decl][discard]") {
    SECTION("var _, important = getResult()") {
        auto fixture = createParserFixture("var _, important = getResult()");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *varDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(!varDecl->isConst());
        REQUIRE(varDecl->names.size() == 2);
        REQUIRE(varDecl->type == nullptr);
        REQUIRE(varDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier _)
  (Identifier important)
  (CallExpr
    (Identifier getResult))))");
    }

    SECTION("const _, _, value = getTriple()") {
        auto fixture = createParserFixture("const _, _, value = getTriple()");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *constDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(constDecl->isConst());
        REQUIRE(constDecl->names.size() == 3);
        REQUIRE(constDecl->type == nullptr);
        REQUIRE(constDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier _)
  (Identifier _)
  (Identifier value)
  (CallExpr
    (Identifier getTriple))))");
    }
}

TEST_CASE("Variable Declaration Parsing - Semicolon Handling", "[parser][statements][var-decl][semicolon]") {
    SECTION("var x = 42;") {
        auto fixture = createParserFixture("var x = 42;");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier x)
  (Int 42)))");

        // Should consume the semicolon
        REQUIRE(fixture->current().kind == TokenKind::EoF);
    }

    SECTION("const PI = 3.14; break") {
        auto fixture = createParserFixture("const PI = 3.14; break");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier PI)
  (Float 3.14)))");

        // Should be positioned after semicolon
        REQUIRE(fixture->current().kind == TokenKind::Break);
    }

    SECTION("var x = 42 break") {
        auto fixture = createParserFixture("var x = 42 break");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier x)
  (Int 42)))");

        // Should stop before 'break' keyword
        REQUIRE(fixture->current().kind == TokenKind::Break);
    }
}

TEST_CASE("Variable Declaration Parsing - Error Cases", "[parser][statements][var-decl][errors]") {
    SECTION("var x - missing type and initializer") {
        auto fixture = createParserFixture("var x");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("const y - missing type and initializer") {
        auto fixture = createParserFixture("const y");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("auto z - missing type and initializer") {
        auto fixture = createParserFixture("auto z");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("var - missing variable name") {
        auto fixture = createParserFixture("var = 42");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("const - missing variable name") {
        auto fixture = createParserFixture("const : i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("var x, - trailing comma without more names") {
        auto fixture = createParserFixture("var x, = 42");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *varDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(varDecl->names.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier x)
  (Int 42)))");
    }

    SECTION("var x: - missing type after colon") {
        auto fixture = createParserFixture("var x: = 42");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("var x = - missing initializer expression") {
        auto fixture = createParserFixture("var x = ");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("var x: 123invalid") {
        auto fixture = createParserFixture("var x: 123invalid");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Variable Declaration Parsing - Visibility Modifiers", "[parser][declarations][var-decl][visibility]") {
    SECTION("pub var globalCounter = 0") {
        auto fixture = createParserFixture("pub var globalCounter = 0");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);
        REQUIRE((stmt->flags & flgPublic) != 0);
        REQUIRE((stmt->flags & flgExtern) == 0);

        auto *varDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(varDecl->names.size() == 1);
        REQUIRE(varDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier globalCounter)
  (Int 0)))");
    }

    SECTION("extern var errno: i32") {
        auto fixture = createParserFixture("extern var errno: i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);
        REQUIRE((stmt->flags & flgExtern) != 0);
        REQUIRE((stmt->flags & flgPublic) == 0);

        auto *varDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(varDecl->names.size() == 1);
        REQUIRE(varDecl->type != nullptr);
        REQUIRE(varDecl->initializer == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier errno)
  (Type i32)))");
    }

    SECTION("pub const MAX_SIZE = 1024") {
        auto fixture = createParserFixture("pub const MAX_SIZE = 1024");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);
        REQUIRE((stmt->flags & flgPublic) != 0);
        REQUIRE((stmt->flags & flgConst) != 0);

        auto *varDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(varDecl->names.size() == 1);
        REQUIRE(varDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier MAX_SIZE)
  (Int 1024)))");
    }

    SECTION("@deprecated pub var legacyVar = 42") {
        auto fixture = createParserFixture("@deprecated pub var legacyVar = 42");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);
        REQUIRE((stmt->flags & flgPublic) != 0);
        REQUIRE(stmt->hasAttributes());
        REQUIRE(stmt->getAttributeCount() == 1);

        auto *varDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(varDecl->names.size() == 1);
        REQUIRE(varDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier legacyVar)
  (Int 42)))");
    }
}

TEST_CASE("Variable Declaration Parsing - Extern Validation Errors", "[parser][declarations][var-decl][extern][errors]") {
    SECTION("extern var without type annotation") {
        auto fixture = createParserFixture("extern var counter");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("extern var with initializer") {
        auto fixture = createParserFixture("extern var errno: i32 = 42");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("extern const with initializer") {
        auto fixture = createParserFixture("extern const MAX_SIZE: i32 = 1024");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("extern var without type and with initializer") {
        auto fixture = createParserFixture("extern var value = 100");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Variable Declaration Parsing - Complex Expressions", "[parser][statements][var-decl][complex]") {
    SECTION("var result = add(1, 2)") {
        auto fixture = createParserFixture("var result = add(1, 2)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *varDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(!varDecl->isConst());
        REQUIRE(varDecl->names.size() == 1);
        REQUIRE(varDecl->type == nullptr);
        REQUIRE(varDecl->initializer != nullptr);
        REQUIRE(varDecl->initializer->kind == astCallExpr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier result)
  (CallExpr
    (Identifier add)
    (Int 1)
    (Int 2))))");
    }

    SECTION("const sum: i32 = x + y") {
        auto fixture = createParserFixture("const sum: i32 = x + y");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *constDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(constDecl->isConst());
        REQUIRE(constDecl->names.size() == 1);
        REQUIRE(constDecl->type != nullptr);
        REQUIRE(constDecl->initializer != nullptr);
        REQUIRE(constDecl->initializer->kind == astBinaryExpr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier sum)
  (Type i32)
  (BinaryExpr +
    (Identifier x)
    (Identifier y))))");
    }

    SECTION("auto flag = x > 10 && y < 20") {
        auto fixture = createParserFixture("auto flag = x > 10 && y < 20");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *autoDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(!autoDecl->isConst());
        REQUIRE(autoDecl->names.size() == 1);
        REQUIRE(autoDecl->type == nullptr);
        REQUIRE(autoDecl->initializer != nullptr);
        REQUIRE(autoDecl->initializer->kind == astBinaryExpr);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier flag)
  (BinaryExpr &&
    (BinaryExpr >
      (Identifier x)
      (Int 10))
    (BinaryExpr <
      (Identifier y)
      (Int 20)))))");
    }
}

TEST_CASE("Variable Declaration Parsing - Statement Dispatch", "[parser][statements][var-decl][dispatch]") {
    SECTION("parseDeclaration() correctly dispatches var declarations") {
        auto fixture = createParserFixture("var x = 5");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);
    }

    SECTION("parseDeclaration() correctly dispatches const declarations") {
        auto fixture = createParserFixture("const Y = 10");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *constDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(constDecl->isConst());
    }

    SECTION("parseDeclaration() correctly dispatches auto declarations") {
        auto fixture = createParserFixture("auto z = true");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *autoDecl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(!autoDecl->isConst());
    }
}
