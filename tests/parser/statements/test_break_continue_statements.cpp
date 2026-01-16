#include "catch2.hpp"
#include "../../parser_test_utils.hpp"
#include "cxy/ast/statements.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::test;

TEST_CASE("Break Statement Parsing", "[parser][statements][break-stmt]") {
    SECTION("Simple break statement without semicolon") {
        auto fixture = createParserFixture("break");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBreakStmt);

        auto *breakStmt = static_cast<BreakStatementNode *>(stmt);
        REQUIRE_AST_MATCHES(stmt, "(BreakStmt)");
    }

    SECTION("Simple break statement with semicolon") {
        auto fixture = createParserFixture("break;");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBreakStmt);

        auto *breakStmt = static_cast<BreakStatementNode *>(stmt);
        REQUIRE_AST_MATCHES(stmt, "(BreakStmt)");
    }

    SECTION("Break statement followed by other tokens") {
        auto fixture = createParserFixture("break if");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBreakStmt);
        REQUIRE_AST_MATCHES(stmt, "(BreakStmt)");

        // Should be positioned at the next token after break
        REQUIRE(fixture->current().kind == TokenKind::If);
    }

    SECTION("Break statement with semicolon followed by other tokens") {
        auto fixture = createParserFixture("break; foo()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBreakStmt);
        REQUIRE_AST_MATCHES(stmt, "(BreakStmt)");

        // Should be positioned after semicolon
        REQUIRE(fixture->current().kind == TokenKind::Ident);
        REQUIRE(fixture->current().value.stringValue.view() == "foo");
    }
}

TEST_CASE("Continue Statement Parsing", "[parser][statements][continue-stmt]") {
    SECTION("Simple continue statement without semicolon") {
        auto fixture = createParserFixture("continue");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astContinueStmt);

        auto *continueStmt = static_cast<ContinueStatementNode *>(stmt);
        REQUIRE_AST_MATCHES(stmt, "(ContinueStmt)");
    }

    SECTION("Simple continue statement with semicolon") {
        auto fixture = createParserFixture("continue;");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astContinueStmt);

        auto *continueStmt = static_cast<ContinueStatementNode *>(stmt);
        REQUIRE_AST_MATCHES(stmt, "(ContinueStmt)");
    }

    SECTION("Continue statement followed by other tokens") {
        auto fixture = createParserFixture("continue while");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astContinueStmt);
        REQUIRE_AST_MATCHES(stmt, "(ContinueStmt)");

        // Should be positioned at the next token after continue
        REQUIRE(fixture->current().kind == TokenKind::While);
    }

    SECTION("Continue statement with semicolon followed by other tokens") {
        auto fixture = createParserFixture("continue; x = 5");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astContinueStmt);
        REQUIRE_AST_MATCHES(stmt, "(ContinueStmt)");

        // Should be positioned after semicolon
        REQUIRE(fixture->current().kind == TokenKind::Ident);
        REQUIRE(fixture->current().value.stringValue.view() == "x");
    }
}

TEST_CASE("Statement Dispatch to Expression Statement", "[parser][statements][dispatch]") {
    SECTION("Function call should parse as expression statement") {
        auto fixture = createParserFixture("foo()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astExprStmt);

        auto *exprStmt = static_cast<ExpressionStatementNode *>(stmt);
        REQUIRE(exprStmt->expression != nullptr);
        REQUIRE(exprStmt->expression->kind == astCallExpr);

        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (CallExpr (Identifier foo)))");
    }

    SECTION("Assignment should parse as expression statement") {
        auto fixture = createParserFixture("x = 42");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astExprStmt);

        auto *exprStmt = static_cast<ExpressionStatementNode *>(stmt);
        REQUIRE(exprStmt->expression != nullptr);
        REQUIRE(exprStmt->expression->kind == astAssignmentExpr);

        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (AssignmentExpr = (Identifier x) (Int 42)))");
    }

    SECTION("Identifier should parse as expression statement") {
        auto fixture = createParserFixture("identifier");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astExprStmt);

        auto *exprStmt = static_cast<ExpressionStatementNode *>(stmt);
        REQUIRE(exprStmt->expression != nullptr);
        REQUIRE(exprStmt->expression->kind == ast::astIdentifier);

        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (Identifier identifier))");
    }
}

TEST_CASE("Statement Boundary Detection", "[parser][statements][boundaries]") {
    SECTION("Statement boundary without semicolon - end of input") {
        auto fixture = createParserFixture("break");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBreakStmt);
        REQUIRE(fixture->current().kind == TokenKind::EoF);
    }

    SECTION("Statement boundary with semicolon") {
        auto fixture = createParserFixture("continue; break");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astContinueStmt);
        REQUIRE_AST_MATCHES(stmt, "(ContinueStmt)");

        // Should be positioned after semicolon
        REQUIRE(fixture->current().kind == TokenKind::Break);
    }

    SECTION("Statement boundary before statement keyword") {
        auto fixture = createParserFixture("break continue");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBreakStmt);
        REQUIRE_AST_MATCHES(stmt, "(BreakStmt)");

        // Should stop before 'continue' keyword
        REQUIRE(fixture->current().kind == TokenKind::Continue);
    }

    SECTION("Statement boundary before block") {
        auto fixture = createParserFixture("continue { foo(); }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astContinueStmt);
        REQUIRE_AST_MATCHES(stmt, "(ContinueStmt)");

        // Should stop before opening brace
        REQUIRE(fixture->current().kind == TokenKind::LBrace);
    }
}

TEST_CASE("Statement Error Cases", "[parser][statements][errors]") {
    SECTION("Empty input") {
        auto fixture = createParserFixture("");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Unexpected token") {
        auto fixture = createParserFixture("]");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Break with unexpected following token") {
        auto fixture = createParserFixture("break ]");
        auto *stmt = fixture->parseStatement();

        // Should successfully parse the break statement
        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBreakStmt);
        REQUIRE_AST_MATCHES(stmt, "(BreakStmt)");

        // But should be positioned at the unexpected token
        REQUIRE(fixture->current().kind == TokenKind::RBracket);
    }

    SECTION("Continue with unexpected following token") {
        auto fixture = createParserFixture("continue )");
        auto *stmt = fixture->parseStatement();

        // Should successfully parse the continue statement
        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astContinueStmt);
        REQUIRE_AST_MATCHES(stmt, "(ContinueStmt)");

        // But should be positioned at the unexpected token
        REQUIRE(fixture->current().kind == TokenKind::RParen);
    }
}