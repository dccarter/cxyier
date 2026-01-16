#include "catch2.hpp"
#include "../../parser_test_utils.hpp"
#include "cxy/ast/statements.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::test;

TEST_CASE("Block Statement Parsing", "[parser][statements][block-stmt]") {
    SECTION("Empty block statement") {
        auto fixture = createParserFixture("{}");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);

        auto *blockStmt = static_cast<BlockStatementNode *>(stmt);
        REQUIRE(blockStmt->statements.size() == 0);

        REQUIRE_AST_MATCHES(stmt, "(BlockStmt)");
    }

    SECTION("Single statement block") {
        auto fixture = createParserFixture("{ foo(); }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);

        auto *blockStmt = static_cast<BlockStatementNode *>(stmt);
        REQUIRE(blockStmt->statements.size() == 1);
        REQUIRE(blockStmt->statements[0]->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, "(BlockStmt (ExprStmt (CallExpr (Identifier foo))))");
    }

    SECTION("Multiple statement block") {
        auto fixture = createParserFixture("{ x = 42; y = 24; }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);

        auto *blockStmt = static_cast<BlockStatementNode *>(stmt);
        REQUIRE(blockStmt->statements.size() == 2);
        REQUIRE(blockStmt->statements[0]->kind == astExprStmt);
        REQUIRE(blockStmt->statements[1]->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, "(BlockStmt (ExprStmt (AssignmentExpr = (Identifier x) (Int 42))) (ExprStmt (AssignmentExpr = (Identifier y) (Int 24))))");
    }

    SECTION("Block with break and continue statements") {
        auto fixture = createParserFixture("{ break; continue; foo(); }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);

        auto *blockStmt = static_cast<BlockStatementNode *>(stmt);
        REQUIRE(blockStmt->statements.size() == 3);
        REQUIRE(blockStmt->statements[0]->kind == astBreakStmt);
        REQUIRE(blockStmt->statements[1]->kind == astContinueStmt);
        REQUIRE(blockStmt->statements[2]->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, "(BlockStmt (BreakStmt) (ContinueStmt) (ExprStmt (CallExpr (Identifier foo))))");
    }

    SECTION("Block with optional semicolons") {
        auto fixture = createParserFixture("{ foo() bar(); baz }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);

        auto *blockStmt = static_cast<BlockStatementNode *>(stmt);
        REQUIRE(blockStmt->statements.size() == 3);
        
        // All should be expression statements
        REQUIRE(blockStmt->statements[0]->kind == astExprStmt);
        REQUIRE(blockStmt->statements[1]->kind == astExprStmt);
        REQUIRE(blockStmt->statements[2]->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, "(BlockStmt (ExprStmt (CallExpr (Identifier foo))) (ExprStmt (CallExpr (Identifier bar))) (ExprStmt (Identifier baz)))");
    }
}

TEST_CASE("Nested Block Statements", "[parser][statements][block-stmt][nested]") {
    SECTION("Simple nested blocks") {
        auto fixture = createParserFixture("{ { inner(); } outer(); }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);

        auto *blockStmt = static_cast<BlockStatementNode *>(stmt);
        REQUIRE(blockStmt->statements.size() == 2);
        REQUIRE(blockStmt->statements[0]->kind == astBlockStmt);
        REQUIRE(blockStmt->statements[1]->kind == astExprStmt);

        // Check inner block
        auto *innerBlock = static_cast<BlockStatementNode *>(blockStmt->statements[0]);
        REQUIRE(innerBlock->statements.size() == 1);
        REQUIRE(innerBlock->statements[0]->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, "(BlockStmt (BlockStmt (ExprStmt (CallExpr (Identifier inner)))) (ExprStmt (CallExpr (Identifier outer))))");
    }

    SECTION("Deeply nested blocks") {
        auto fixture = createParserFixture("{ { { innermost(); } } }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);

        auto *outerBlock = static_cast<BlockStatementNode *>(stmt);
        REQUIRE(outerBlock->statements.size() == 1);
        REQUIRE(outerBlock->statements[0]->kind == astBlockStmt);

        auto *middleBlock = static_cast<BlockStatementNode *>(outerBlock->statements[0]);
        REQUIRE(middleBlock->statements.size() == 1);
        REQUIRE(middleBlock->statements[0]->kind == astBlockStmt);

        auto *innerBlock = static_cast<BlockStatementNode *>(middleBlock->statements[0]);
        REQUIRE(innerBlock->statements.size() == 1);
        REQUIRE(innerBlock->statements[0]->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, "(BlockStmt (BlockStmt (BlockStmt (ExprStmt (CallExpr (Identifier innermost))))))");
    }

    SECTION("Mixed nested blocks and statements") {
        auto fixture = createParserFixture("{ before(); { nested(); } after(); }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);

        auto *blockStmt = static_cast<BlockStatementNode *>(stmt);
        REQUIRE(blockStmt->statements.size() == 3);
        REQUIRE(blockStmt->statements[0]->kind == astExprStmt);
        REQUIRE(blockStmt->statements[1]->kind == astBlockStmt);
        REQUIRE(blockStmt->statements[2]->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, "(BlockStmt (ExprStmt (CallExpr (Identifier before))) (BlockStmt (ExprStmt (CallExpr (Identifier nested)))) (ExprStmt (CallExpr (Identifier after))))");
    }
}

TEST_CASE("Block Statement Boundary Detection", "[parser][statements][block-stmt][boundaries]") {
    SECTION("Block boundary at end of input") {
        auto fixture = createParserFixture("{ foo(); }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);
        REQUIRE(fixture->current().kind == TokenKind::EoF);
    }

    SECTION("Block followed by other statements") {
        auto fixture = createParserFixture("{ foo(); } bar();");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);
        REQUIRE_AST_MATCHES(stmt, "(BlockStmt (ExprStmt (CallExpr (Identifier foo))))");

        // Should be positioned after the closing brace
        REQUIRE(fixture->current().kind == TokenKind::Ident);
        REQUIRE(fixture->current().value.stringValue.view() == "bar");
    }

    SECTION("Empty block followed by statement") {
        auto fixture = createParserFixture("{} continue;");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);
        REQUIRE_AST_MATCHES(stmt, "(BlockStmt)");

        // Should be positioned after the closing brace
        REQUIRE(fixture->current().kind == TokenKind::Continue);
    }

    SECTION("Block with statement boundary detection inside") {
        auto fixture = createParserFixture("{ foo() bar() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);

        auto *blockStmt = static_cast<BlockStatementNode *>(stmt);
        REQUIRE(blockStmt->statements.size() == 2);
        REQUIRE_AST_MATCHES(stmt, "(BlockStmt (ExprStmt (CallExpr (Identifier foo))) (ExprStmt (CallExpr (Identifier bar))))");
    }
}

TEST_CASE("Block Statement Error Cases", "[parser][statements][block-stmt][errors]") {
    SECTION("Missing opening brace") {
        auto fixture = createParserFixture("foo(); }");
        auto *stmt = fixture->parseStatement();

        // Should parse as expression statement, not block
        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astExprStmt);
        REQUIRE_AST_MATCHES(stmt, "(ExprStmt (CallExpr (Identifier foo)))");

        // Should be positioned at the unexpected closing brace
        REQUIRE(fixture->current().kind == TokenKind::RBrace);
    }

    SECTION("Missing closing brace") {
        auto fixture = createParserFixture("{ foo();");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing closing brace with nested blocks") {
        auto fixture = createParserFixture("{ { inner(); }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Invalid statement in block") {
        auto fixture = createParserFixture("{ foo(); ++ }");
        auto *stmt = fixture->parseStatement();

        // Should parse the valid parts and handle the error
        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);

        auto *blockStmt = static_cast<BlockStatementNode *>(stmt);
        REQUIRE(blockStmt->statements.size() == 1); // Only foo(); should be parsed
        REQUIRE(blockStmt->statements[0]->kind == astExprStmt);

        // Should have reported an error for the invalid "++" expression
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Empty block with extra closing brace") {
        auto fixture = createParserFixture("{ } }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);
        REQUIRE_AST_MATCHES(stmt, "(BlockStmt)");

        // Should be positioned at the unexpected extra closing brace
        REQUIRE(fixture->current().kind == TokenKind::RBrace);
    }

    SECTION("Unmatched opening brace at end") {
        auto fixture = createParserFixture("{");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Block Statement Integration", "[parser][statements][block-stmt][integration]") {
    SECTION("Block containing all statement types") {
        auto fixture = createParserFixture("{ foo(); break; continue; { nested(); } }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);

        auto *blockStmt = static_cast<BlockStatementNode *>(stmt);
        REQUIRE(blockStmt->statements.size() == 4);
        REQUIRE(blockStmt->statements[0]->kind == astExprStmt);
        REQUIRE(blockStmt->statements[1]->kind == astBreakStmt);
        REQUIRE(blockStmt->statements[2]->kind == astContinueStmt);
        REQUIRE(blockStmt->statements[3]->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, "(BlockStmt (ExprStmt (CallExpr (Identifier foo))) (BreakStmt) (ContinueStmt) (BlockStmt (ExprStmt (CallExpr (Identifier nested)))))");
    }

    SECTION("Large block with many statements") {
        auto fixture = createParserFixture("{ a(); b(); c(); d(); e(); f(); g(); h(); }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);

        auto *blockStmt = static_cast<BlockStatementNode *>(stmt);
        REQUIRE(blockStmt->statements.size() == 8);
        
        // All should be expression statements
        for (size_t i = 0; i < 8; ++i) {
            REQUIRE(blockStmt->statements[i]->kind == astExprStmt);
        }
    }

    SECTION("Complex assignment expressions in block") {
        auto fixture = createParserFixture("{ x = y + z; arr[i] = val; obj.field *= 2; }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astBlockStmt);

        auto *blockStmt = static_cast<BlockStatementNode *>(stmt);
        REQUIRE(blockStmt->statements.size() == 3);
        
        // All should be expression statements with assignment expressions
        for (size_t i = 0; i < 3; ++i) {
            REQUIRE(blockStmt->statements[i]->kind == astExprStmt);
            auto *exprStmt = static_cast<ExpressionStatementNode *>(blockStmt->statements[i]);
            // Should contain assignment expressions (simplified check)
            REQUIRE(exprStmt->expression != nullptr);
        }
    }
}