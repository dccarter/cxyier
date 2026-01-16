#include "catch2.hpp"
#include "../../parser_test_utils.hpp"
#include "cxy/ast/statements.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::test;

TEST_CASE("Defer Statement Parsing", "[parser][statements][defer-stmt]") {
    SECTION("Defer with expression statement") {
        auto fixture = createParserFixture("defer cleanup()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astDeferStmt);

        auto *deferStmt = static_cast<DeferStatementNode *>(stmt);
        REQUIRE(deferStmt->statement != nullptr);
        REQUIRE(deferStmt->statement->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, "(DeferStmt (ExprStmt (CallExpr (Identifier cleanup))))");
    }

    SECTION("Defer with block statement") {
        auto fixture = createParserFixture("defer { cleanup(); logExit(); }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astDeferStmt);

        auto *deferStmt = static_cast<DeferStatementNode *>(stmt);
        REQUIRE(deferStmt->statement != nullptr);
        REQUIRE(deferStmt->statement->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, "(DeferStmt (BlockStmt (ExprStmt (CallExpr (Identifier cleanup))) (ExprStmt (CallExpr (Identifier logExit)))))");
    }

    SECTION("Defer with assignment statement") {
        auto fixture = createParserFixture("defer x = 0");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astDeferStmt);

        auto *deferStmt = static_cast<DeferStatementNode *>(stmt);
        REQUIRE(deferStmt->statement != nullptr);
        REQUIRE(deferStmt->statement->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, "(DeferStmt (ExprStmt (AssignmentExpr = (Identifier x) (Int 0))))");
    }

    SECTION("Defer followed by other statements") {
        auto fixture = createParserFixture("defer cleanup() foo()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astDeferStmt);
        REQUIRE_AST_MATCHES(stmt, "(DeferStmt (ExprStmt (CallExpr (Identifier cleanup))))");

        // Should be positioned at the next token
        REQUIRE(fixture->current().kind == TokenKind::Ident);
        REQUIRE(fixture->current().value.stringValue.view() == "foo");
    }
}

TEST_CASE("Return Statement Parsing", "[parser][statements][return-stmt]") {
    SECTION("Return without value or semicolon") {
        auto fixture = createParserFixture("return");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astReturnStmt);

        auto *returnStmt = static_cast<ReturnStatementNode *>(stmt);
        REQUIRE(returnStmt->expression == nullptr);

        REQUIRE_AST_MATCHES(stmt, "(ReturnStmt)");
    }

    SECTION("Return without value with semicolon") {
        auto fixture = createParserFixture("return;");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astReturnStmt);

        auto *returnStmt = static_cast<ReturnStatementNode *>(stmt);
        REQUIRE(returnStmt->expression == nullptr);

        REQUIRE_AST_MATCHES(stmt, "(ReturnStmt)");
    }

    SECTION("Return with integer literal") {
        auto fixture = createParserFixture("return 42");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astReturnStmt);

        auto *returnStmt = static_cast<ReturnStatementNode *>(stmt);
        REQUIRE(returnStmt->expression != nullptr);
        REQUIRE(returnStmt->expression->kind == ast::astInt);

        REQUIRE_AST_MATCHES(stmt, "(ReturnStmt (Int 42))");
    }

    SECTION("Return with expression and semicolon") {
        auto fixture = createParserFixture("return getValue();");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astReturnStmt);

        auto *returnStmt = static_cast<ReturnStatementNode *>(stmt);
        REQUIRE(returnStmt->expression != nullptr);
        REQUIRE(returnStmt->expression->kind == astCallExpr);

        REQUIRE_AST_MATCHES(stmt, "(ReturnStmt (CallExpr (Identifier getValue)))");
    }

    SECTION("Return with complex expression") {
        auto fixture = createParserFixture("return x + y * 2");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astReturnStmt);

        auto *returnStmt = static_cast<ReturnStatementNode *>(stmt);
        REQUIRE(returnStmt->expression != nullptr);
        REQUIRE(returnStmt->expression->kind == astBinaryExpr);

        REQUIRE_AST_MATCHES(stmt, "(ReturnStmt (BinaryExpr + (Identifier x) (BinaryExpr * (Identifier y) (Int 2))))");
    }

    SECTION("Return followed by other statements") {
        auto fixture = createParserFixture("return 42 foo()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astReturnStmt);
        REQUIRE_AST_MATCHES(stmt, "(ReturnStmt (Int 42))");

        // Should be positioned at the next token
        REQUIRE(fixture->current().kind == TokenKind::Ident);
        REQUIRE(fixture->current().value.stringValue.view() == "foo");
    }
}

TEST_CASE("Yield Statement Parsing", "[parser][statements][yield-stmt]") {
    SECTION("Yield without value or semicolon") {
        auto fixture = createParserFixture("yield");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astYieldStmt);

        auto *yieldStmt = static_cast<YieldStatementNode *>(stmt);
        REQUIRE(yieldStmt->expression == nullptr);

        REQUIRE_AST_MATCHES(stmt, "(YieldStmt)");
    }

    SECTION("Yield without value with semicolon") {
        auto fixture = createParserFixture("yield;");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astYieldStmt);

        auto *yieldStmt = static_cast<YieldStatementNode *>(stmt);
        REQUIRE(yieldStmt->expression == nullptr);

        REQUIRE_AST_MATCHES(stmt, "(YieldStmt)");
    }

    SECTION("Yield with string literal") {
        auto fixture = createParserFixture("yield \"hello\"");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astYieldStmt);

        auto *yieldStmt = static_cast<YieldStatementNode *>(stmt);
        REQUIRE(yieldStmt->expression != nullptr);
        REQUIRE(yieldStmt->expression->kind == ast::astString);

        REQUIRE_AST_MATCHES(stmt, "(YieldStmt (String \"hello\"))");
    }

    SECTION("Yield with expression and semicolon") {
        auto fixture = createParserFixture("yield computeNext();");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astYieldStmt);

        auto *yieldStmt = static_cast<YieldStatementNode *>(stmt);
        REQUIRE(yieldStmt->expression != nullptr);
        REQUIRE(yieldStmt->expression->kind == astCallExpr);

        REQUIRE_AST_MATCHES(stmt, "(YieldStmt (CallExpr (Identifier computeNext)))");
    }

    SECTION("Yield with complex expression") {
        auto fixture = createParserFixture("yield arr[index] + offset");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astYieldStmt);

        auto *yieldStmt = static_cast<YieldStatementNode *>(stmt);
        REQUIRE(yieldStmt->expression != nullptr);
        REQUIRE(yieldStmt->expression->kind == astBinaryExpr);

        REQUIRE_AST_MATCHES(stmt, "(YieldStmt (BinaryExpr + (IndexExpr (Identifier arr) (Identifier index)) (Identifier offset)))");
    }

    SECTION("Yield followed by other statements") {
        auto fixture = createParserFixture("yield value break");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astYieldStmt);
        REQUIRE_AST_MATCHES(stmt, "(YieldStmt (Identifier value))");

        // Should be positioned at the next token
        REQUIRE(fixture->current().kind == TokenKind::Break);
    }
}

TEST_CASE("Statement Boundary Detection for Defer/Return/Yield", "[parser][statements][boundaries]") {
    SECTION("Defer statement boundary at end of input") {
        auto fixture = createParserFixture("defer cleanup()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astDeferStmt);
        REQUIRE(fixture->current().kind == TokenKind::EoF);
    }

    SECTION("Return statement boundary without semicolon") {
        auto fixture = createParserFixture("return 42 if");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astReturnStmt);
        REQUIRE_AST_MATCHES(stmt, "(ReturnStmt (Int 42))");

        // Should stop before 'if' keyword
        REQUIRE(fixture->current().kind == TokenKind::If);
    }

    SECTION("Yield statement boundary with semicolon") {
        auto fixture = createParserFixture("yield value; continue;");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astYieldStmt);
        REQUIRE_AST_MATCHES(stmt, "(YieldStmt (Identifier value))");

        // Should be positioned after semicolon
        REQUIRE(fixture->current().kind == TokenKind::Continue);
    }

    SECTION("Defer with block boundary") {
        auto fixture = createParserFixture("defer { cleanup(); } return");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astDeferStmt);
        REQUIRE_AST_MATCHES(stmt, "(DeferStmt (BlockStmt (ExprStmt (CallExpr (Identifier cleanup)))))");

        // Should stop after block
        REQUIRE(fixture->current().kind == TokenKind::Return);
    }
}

TEST_CASE("Error Cases for Defer/Return/Yield", "[parser][statements][errors]") {
    SECTION("Defer without statement") {
        auto fixture = createParserFixture("defer");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Defer with invalid statement") {
        auto fixture = createParserFixture("defer ++");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Return with invalid expression") {
        auto fixture = createParserFixture("return ++");
        auto *stmt = fixture->parseStatement();

        // Should create return statement with no expression (parser continues after expression parse fails)
        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astReturnStmt);
        
        auto *returnStmt = static_cast<ReturnStatementNode *>(stmt);
        REQUIRE(returnStmt->expression == nullptr);
        
        // Should have errors from failed expression parsing
        REQUIRE(fixture->hasErrors());
        
        // Should be positioned at EOF since ++ gets consumed during failed unary expression parsing
        REQUIRE(fixture->current().kind == TokenKind::EoF);
    }

    SECTION("Yield with invalid expression") {
        auto fixture = createParserFixture("yield ]]");
        auto *stmt = fixture->parseStatement();

        // Should create yield statement with no expression (parser continues after expression parse fails)
        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astYieldStmt);
        
        auto *yieldStmt = static_cast<YieldStatementNode *>(stmt);
        REQUIRE(yieldStmt->expression == nullptr);
        
        // Should have errors from failed expression parsing
        REQUIRE(fixture->hasErrors());
        
        // Should be positioned at first ] since expression parsing stops at invalid token
        REQUIRE(fixture->current().kind == TokenKind::RBracket);
    }

    SECTION("Return followed by unexpected token") {
        auto fixture = createParserFixture("return 42 ]");
        auto *stmt = fixture->parseStatement();

        // Should successfully parse the return statement
        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astReturnStmt);
        REQUIRE_AST_MATCHES(stmt, "(ReturnStmt (Int 42))");

        // But should be positioned at the unexpected token
        REQUIRE(fixture->current().kind == TokenKind::RBracket);
    }

    SECTION("Defer with valid statement followed by error") {
        auto fixture = createParserFixture("defer cleanup() ]");
        auto *stmt = fixture->parseStatement();

        // Should successfully parse the defer statement
        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astDeferStmt);
        REQUIRE_AST_MATCHES(stmt, "(DeferStmt (ExprStmt (CallExpr (Identifier cleanup))))");

        // But should be positioned at the unexpected token
        REQUIRE(fixture->current().kind == TokenKind::RBracket);
    }
}

TEST_CASE("Integration Tests for Defer/Return/Yield", "[parser][statements][integration]") {
    SECTION("Defer with all statement types") {
        auto fixture = createParserFixture("defer { foo(); break; return 42; }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astDeferStmt);

        auto *deferStmt = static_cast<DeferStatementNode *>(stmt);
        REQUIRE(deferStmt->statement != nullptr);
        REQUIRE(deferStmt->statement->kind == astBlockStmt);

        auto *blockStmt = static_cast<BlockStatementNode *>(deferStmt->statement);
        REQUIRE(blockStmt->statements.size() == 3);
        REQUIRE(blockStmt->statements[0]->kind == astExprStmt);
        REQUIRE(blockStmt->statements[1]->kind == astBreakStmt);
        REQUIRE(blockStmt->statements[2]->kind == astReturnStmt);

        REQUIRE_AST_MATCHES(stmt, "(DeferStmt (BlockStmt (ExprStmt (CallExpr (Identifier foo))) (BreakStmt) (ReturnStmt (Int 42))))");
    }

    SECTION("Return with complex nested expression") {
        auto fixture = createParserFixture("return obj.method(arr[i], x + y)");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astReturnStmt);

        auto *returnStmt = static_cast<ReturnStatementNode *>(stmt);
        REQUIRE(returnStmt->expression != nullptr);
        REQUIRE(returnStmt->expression->kind == astCallExpr);

        REQUIRE_AST_MATCHES(stmt, "(ReturnStmt (CallExpr (MemberExpr . (Identifier obj) (Identifier method)) (IndexExpr (Identifier arr) (Identifier i)) (BinaryExpr + (Identifier x) (Identifier y))))");
    }

    SECTION("Yield with assignment expression") {
        auto fixture = createParserFixture("yield result = compute(input)");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astYieldStmt);

        auto *yieldStmt = static_cast<YieldStatementNode *>(stmt);
        REQUIRE(yieldStmt->expression != nullptr);
        REQUIRE(yieldStmt->expression->kind == astAssignmentExpr);

        REQUIRE_AST_MATCHES(stmt, "(YieldStmt (AssignmentExpr = (Identifier result) (CallExpr (Identifier compute) (Identifier input))))");
    }

    SECTION("Multiple defer/return/yield statements") {
        auto fixture = createParserFixture("defer cleanup1(); return getValue(); yield result;");
        
        // Parse first statement (defer)
        auto *stmt1 = fixture->parseStatement();
        REQUIRE(stmt1 != nullptr);
        REQUIRE(stmt1->kind == astDeferStmt);
        REQUIRE_AST_MATCHES(stmt1, "(DeferStmt (ExprStmt (CallExpr (Identifier cleanup1))))");

        // Parse second statement (return)
        auto *stmt2 = fixture->parseStatement();
        REQUIRE(stmt2 != nullptr);
        REQUIRE(stmt2->kind == astReturnStmt);
        REQUIRE_AST_MATCHES(stmt2, "(ReturnStmt (CallExpr (Identifier getValue)))");

        // Parse third statement (yield)
        auto *stmt3 = fixture->parseStatement();
        REQUIRE(stmt3 != nullptr);
        REQUIRE(stmt3->kind == astYieldStmt);
        REQUIRE_AST_MATCHES(stmt3, "(YieldStmt (Identifier result))");
    }
}