#include "catch2.hpp"
#include "../../parser_test_utils.hpp"
#include "cxy/ast/statements.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::test;

TEST_CASE("Expression Statement Parsing", "[parser][statements][expression-stmt]") {
    SECTION("Simple function call statement without semicolon") {
        auto fixture = createParserFixture("foo()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astExprStmt);

        auto *exprStmt = static_cast<ExpressionStatementNode *>(stmt);
        REQUIRE(exprStmt->expression != nullptr);
        REQUIRE(exprStmt->expression->kind == astCallExpr);

        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (CallExpr (Identifier foo)))");
    }

    SECTION("Simple function call statement with semicolon") {
        auto fixture = createParserFixture("foo();");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astExprStmt);

        auto *exprStmt = static_cast<ExpressionStatementNode *>(stmt);
        REQUIRE(exprStmt->expression != nullptr);
        REQUIRE(exprStmt->expression->kind == astCallExpr);

        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (CallExpr (Identifier foo)))");
    }

    SECTION("Assignment statement without semicolon") {
        auto fixture = createParserFixture("x = 42");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astExprStmt);

        auto *exprStmt = static_cast<ExpressionStatementNode *>(stmt);
        REQUIRE(exprStmt->expression != nullptr);
        REQUIRE(exprStmt->expression->kind == astAssignmentExpr);

        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (AssignmentExpr = (Identifier x) (Int 42)))");
    }

    SECTION("Assignment statement with semicolon") {
        auto fixture = createParserFixture("x = 42;");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (AssignmentExpr = (Identifier x) (Int 42)))");
    }

    SECTION("Compound assignment statement") {
        auto fixture = createParserFixture("counter += 1");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (AssignmentExpr += (Identifier counter) (Int 1)))");
    }

    SECTION("Increment statement") {
        auto fixture = createParserFixture("++counter");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (UnaryExpr ++ (Identifier counter)))");
    }

    SECTION("Method chain statement") {
        auto fixture = createParserFixture("obj.method().chain()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astExprStmt);

        auto *exprStmt = static_cast<ExpressionStatementNode *>(stmt);
        REQUIRE(exprStmt->expression != nullptr);
        REQUIRE(exprStmt->expression->kind == astCallExpr);

        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (CallExpr (MemberExpr . (CallExpr (MemberExpr . (Identifier obj) (Identifier method))) (Identifier chain))))");
    }

    SECTION("Macro call statement") {
        auto fixture = createParserFixture("println!(\"Hello\")");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (MacroCallExpr (Identifier println) (String \"Hello\")))");
    }

    SECTION("Complex expression statement") {
        auto fixture = createParserFixture("getData().transform(mapper).filter(predicate).save()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astExprStmt);

        auto *exprStmt = static_cast<ExpressionStatementNode *>(stmt);
        REQUIRE(exprStmt->expression != nullptr);

        // Should parse as chained method calls
        REQUIRE(exprStmt->expression->kind == astCallExpr);
    }
}

TEST_CASE("Expression Statement Boundary Detection", "[parser][statements][boundaries]") {
    SECTION("Statement boundary without semicolon - end of input") {
        auto fixture = createParserFixture("foo()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(fixture->current().kind == TokenKind::EoF);
    }

    SECTION("Statement boundary with semicolon") {
        auto fixture = createParserFixture("foo(); bar()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (CallExpr (Identifier foo)))");

        // Should be positioned after semicolon
        REQUIRE(fixture->current().kind == TokenKind::Ident);
        REQUIRE(fixture->current().value.stringValue.view() == "bar");
    }

    SECTION("Statement boundary before statement keyword") {
        auto fixture = createParserFixture("foo() if");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (CallExpr (Identifier foo)))");

        // Should stop before 'if' keyword
        REQUIRE(fixture->current().kind == TokenKind::If);
    }

    SECTION("Statement boundary before block") {
        auto fixture = createParserFixture("foo() { bar(); }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (CallExpr (Identifier foo)))");

        // Should stop before opening brace
        REQUIRE(fixture->current().kind == TokenKind::LBrace);
    }
}

TEST_CASE("Expression Statement Error Cases", "[parser][statements][errors]") {
    SECTION("Empty input") {
        auto fixture = createParserFixture("");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Invalid expression") {
        auto fixture = createParserFixture("++");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Unexpected token after valid expression") {
        auto fixture = createParserFixture("foo() ]");
        auto *stmt = fixture->parseStatement();

        // Should successfully parse the function call
        REQUIRE(stmt != nullptr);
        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (CallExpr (Identifier foo)))");

        // But should be positioned at the unexpected token
        REQUIRE(fixture->current().kind == TokenKind::RBracket);
    }
}
