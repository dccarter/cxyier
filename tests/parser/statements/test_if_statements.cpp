#include "catch2.hpp"
#include "../../parser_test_utils.hpp"
#include "cxy/ast/statements.hpp"
#include "cxy/ast/declarations.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::test;

TEST_CASE("If Statement Parsing - Basic Expression Conditions", "[parser][statements][if-stmt]") {
    SECTION("if true { }") {
        auto fixture = createParserFixture("if true { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->condition->kind == astBool);
        REQUIRE(ifStmt->thenStatement != nullptr);
        REQUIRE(ifStmt->thenStatement->kind == astBlockStmt);
        REQUIRE(ifStmt->elseStatement == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (Bool true)
  (BlockStmt)))");
    }

    SECTION("if false { println(\"false\") }") {
        auto fixture = createParserFixture("if false { println(\"false\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->condition->kind == astBool);
        REQUIRE(ifStmt->thenStatement != nullptr);
        REQUIRE(ifStmt->thenStatement->kind == astBlockStmt);
        REQUIRE(ifStmt->elseStatement == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (Bool false)
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier println)
        (String "false"))))))");
    }

    SECTION("if x > 10 { return x }") {
        auto fixture = createParserFixture("if x > 10 { return x }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->condition->kind == astBinaryExpr);
        REQUIRE(ifStmt->thenStatement != nullptr);
        REQUIRE(ifStmt->thenStatement->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (BinaryExpr >
    (Identifier x)
    (Int 10))
  (BlockStmt
    (ReturnStmt
      (Identifier x)))))");
    }
}

TEST_CASE("If Statement Parsing - Parenthesized Conditions", "[parser][statements][if-stmt][parens]") {
    SECTION("if (true) println(\"hello\")") {
        auto fixture = createParserFixture("if (true) println(\"hello\")");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->condition->kind == astBool);
        REQUIRE(ifStmt->thenStatement != nullptr);
        REQUIRE(ifStmt->thenStatement->kind == astExprStmt);
        REQUIRE(ifStmt->elseStatement == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (Bool true)
  (ExprStmt
    (CallExpr
      (Identifier println)
      (String "hello")))))");
    }

    SECTION("if (x == 42) return true") {
        auto fixture = createParserFixture("if (x == 42) return true");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->condition->kind == astBinaryExpr);
        REQUIRE(ifStmt->thenStatement != nullptr);
        REQUIRE(ifStmt->thenStatement->kind == astReturnStmt);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (BinaryExpr ==
    (Identifier x)
    (Int 42))
  (ReturnStmt
    (Bool true))))");
    }

    SECTION("if (ready && active) start()") {
        auto fixture = createParserFixture("if (ready && active) start()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (BinaryExpr &&
    (Identifier ready)
    (Identifier active))
  (ExprStmt
    (CallExpr
      (Identifier start)))))");
    }
}

TEST_CASE("If Statement Parsing - Variable Declaration Conditions", "[parser][statements][if-stmt][var-decl]") {
    SECTION("if const x = getValue() { println(x) }") {
        auto fixture = createParserFixture("if const x = getValue() { println(x) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->condition->kind == astVariableDeclaration);
        REQUIRE(ifStmt->thenStatement != nullptr);
        REQUIRE(ifStmt->thenStatement->kind == astBlockStmt);

        auto *condDecl = static_cast<VariableDeclarationNode *>(ifStmt->condition);
        REQUIRE(condDecl->isConst());
        REQUIRE(condDecl->names.size() == 1);
        REQUIRE(condDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (VariableDeclaration
    (Identifier x)
    (CallExpr
      (Identifier getValue)))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier println)
        (Identifier x))))))");
    }

    SECTION("if var result = compute() { handleResult(result) }") {
        auto fixture = createParserFixture("if var result = compute() { handleResult(result) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->condition->kind == astVariableDeclaration);

        auto *condDecl = static_cast<VariableDeclarationNode *>(ifStmt->condition);
        REQUIRE(!condDecl->isConst());
        REQUIRE(condDecl->names.size() == 1);
        REQUIRE(condDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (VariableDeclaration
    (Identifier result)
    (CallExpr
      (Identifier compute)))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier handleResult)
        (Identifier result))))))");
    }

    SECTION("if auto data: string = load() { process(data) }") {
        auto fixture = createParserFixture("if auto data: string = load() { process(data) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->condition->kind == astVariableDeclaration);

        auto *condDecl = static_cast<VariableDeclarationNode *>(ifStmt->condition);
        REQUIRE(!condDecl->isConst());
        REQUIRE(condDecl->names.size() == 1);
        REQUIRE(condDecl->type != nullptr);
        REQUIRE(condDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (VariableDeclaration
    (Identifier data)
    (Type string)
    (CallExpr
      (Identifier load)))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier process)
        (Identifier data))))))");
    }
}

TEST_CASE("If Statement Parsing - Parenthesized Variable Declaration Conditions", "[parser][statements][if-stmt][parens][var-decl]") {
    SECTION("if (const x = fetch()) handle(x)") {
        auto fixture = createParserFixture("if (const x = fetch()) handle(x)");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->condition->kind == astVariableDeclaration);
        REQUIRE(ifStmt->thenStatement != nullptr);
        REQUIRE(ifStmt->thenStatement->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (VariableDeclaration
    (Identifier x)
    (CallExpr
      (Identifier fetch)))
  (ExprStmt
    (CallExpr
      (Identifier handle)
      (Identifier x)))))");
    }

    SECTION("if (var status = check()) return status") {
        auto fixture = createParserFixture("if (var status = check()) return status");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->condition->kind == astVariableDeclaration);
        REQUIRE(ifStmt->thenStatement != nullptr);
        REQUIRE(ifStmt->thenStatement->kind == astReturnStmt);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (VariableDeclaration
    (Identifier status)
    (CallExpr
      (Identifier check)))
  (ReturnStmt
    (Identifier status))))");
    }
}

TEST_CASE("If Statement Parsing - Else Clauses", "[parser][statements][if-stmt][else]") {
    SECTION("if true { println(\"yes\") } else { println(\"no\") }") {
        auto fixture = createParserFixture("if true { println(\"yes\") } else { println(\"no\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->thenStatement != nullptr);
        REQUIRE(ifStmt->elseStatement != nullptr);
        REQUIRE(ifStmt->elseStatement->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (Bool true)
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier println)
        (String "yes"))))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier println)
        (String "no"))))))");
    }

    SECTION("if (flag) doSomething() else doOther()") {
        auto fixture = createParserFixture("if (flag) doSomething() else doOther()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->thenStatement != nullptr);
        REQUIRE(ifStmt->thenStatement->kind == astExprStmt);
        REQUIRE(ifStmt->elseStatement != nullptr);
        REQUIRE(ifStmt->elseStatement->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (Identifier flag)
  (ExprStmt
    (CallExpr
      (Identifier doSomething)))
  (ExprStmt
    (CallExpr
      (Identifier doOther)))))");
    }
}

TEST_CASE("If Statement Parsing - Else If Chains", "[parser][statements][if-stmt][else-if]") {
    SECTION("if x > 10 { } else if x > 5 { } else { }") {
        auto fixture = createParserFixture("if x > 10 { } else if x > 5 { } else { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->thenStatement != nullptr);
        REQUIRE(ifStmt->elseStatement != nullptr);
        REQUIRE(ifStmt->elseStatement->kind == astIfStmt); // nested if statement

        auto *elseIf = static_cast<IfStatementNode *>(ifStmt->elseStatement);
        REQUIRE(elseIf->condition != nullptr);
        REQUIRE(elseIf->thenStatement != nullptr);
        REQUIRE(elseIf->elseStatement != nullptr);
        REQUIRE(elseIf->elseStatement->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (BinaryExpr >
    (Identifier x)
    (Int 10))
  (BlockStmt)
  (IfStmt
    (BinaryExpr >
      (Identifier x)
      (Int 5))
    (BlockStmt)
    (BlockStmt))))");
    }

    SECTION("if const x = first() { } else if var y = second() { } else { }") {
        auto fixture = createParserFixture("if const x = first() { } else if var y = second() { } else { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->condition->kind == astVariableDeclaration);
        REQUIRE(ifStmt->elseStatement != nullptr);
        REQUIRE(ifStmt->elseStatement->kind == astIfStmt);

        auto *elseIf = static_cast<IfStatementNode *>(ifStmt->elseStatement);
        REQUIRE(elseIf->condition != nullptr);
        REQUIRE(elseIf->condition->kind == astVariableDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (VariableDeclaration
    (Identifier x)
    (CallExpr
      (Identifier first)))
  (BlockStmt)
  (IfStmt
    (VariableDeclaration
      (Identifier y)
      (CallExpr
        (Identifier second)))
    (BlockStmt)
    (BlockStmt))))");
    }
}

TEST_CASE("If Statement Parsing - Error Cases", "[parser][statements][if-stmt][errors]") {
    SECTION("if without condition") {
        auto fixture = createParserFixture("if { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("if without body") {
        auto fixture = createParserFixture("if true");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("if with multiple variable declaration (should fail)") {
        auto fixture = createParserFixture("if var a, b = getTuple() { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("if with variable declaration without initializer") {
        auto fixture = createParserFixture("if var x: i32 { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("if with malformed parentheses") {
        auto fixture = createParserFixture("if (true { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("if condition without body parentheses - missing braces") {
        auto fixture = createParserFixture("if true println()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("else without if") {
        auto fixture = createParserFixture("else { }");
        auto *stmt = fixture->parseStatement();

        // Should parse as expression statement and fail
        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("If Statement Parsing - Complex Conditions", "[parser][statements][if-stmt][complex]") {
    SECTION("if x > 0 && y < 10 || z == 5 { }") {
        auto fixture = createParserFixture("if x > 0 && y < 10 || z == 5 { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->condition->kind == astBinaryExpr);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (BinaryExpr ||
    (BinaryExpr &&
      (BinaryExpr >
        (Identifier x)
        (Int 0))
      (BinaryExpr <
        (Identifier y)
        (Int 10)))
    (BinaryExpr ==
      (Identifier z)
      (Int 5)))
  (BlockStmt)))");
    }

    SECTION("if getValue().isValid() { }") {
        auto fixture = createParserFixture("if getValue().isValid() { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        auto *ifStmt = static_cast<IfStatementNode *>(stmt);
        REQUIRE(ifStmt->condition != nullptr);
        REQUIRE(ifStmt->condition->kind == astCallExpr);

        REQUIRE_AST_MATCHES(stmt, R"((IfStmt
  (CallExpr
    (MemberExpr .
      (CallExpr
        (Identifier getValue))
      (Identifier isValid)))
  (BlockStmt)))");
    }
}

TEST_CASE("If Statement Parsing - Statement Dispatch", "[parser][statements][if-stmt][dispatch]") {
    SECTION("parseStatement() correctly dispatches if statements") {
        auto fixture = createParserFixture("if true { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);
    }

    SECTION("if statement followed by other tokens") {
        auto fixture = createParserFixture("if true { } break");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astIfStmt);

        // Should be positioned at the next token after if statement
        REQUIRE(fixture->current().kind == TokenKind::Break);
    }
}