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

TEST_CASE("While Statement Parsing - Infinite Loops", "[parser][statements][while-stmt]") {
    SECTION("while { }") {
        auto fixture = createParserFixture("while { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition == nullptr); // infinite loop
        REQUIRE(whileStmt->body != nullptr);
        REQUIRE(whileStmt->body->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (BlockStmt)))");
    }

    SECTION("while { println(\"forever\") }") {
        auto fixture = createParserFixture("while { println(\"forever\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition == nullptr);
        REQUIRE(whileStmt->body != nullptr);
        REQUIRE(whileStmt->body->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier println)
        (String "forever"))))))");
    }

    SECTION("while { break }") {
        auto fixture = createParserFixture("while { break }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition == nullptr);
        REQUIRE(whileStmt->body != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (BlockStmt
    (BreakStmt))))");
    }
}

TEST_CASE("While Statement Parsing - Basic Expression Conditions", "[parser][statements][while-stmt]") {
    SECTION("while true { }") {
        auto fixture = createParserFixture("while true { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astBool);
        REQUIRE(whileStmt->body != nullptr);
        REQUIRE(whileStmt->body->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (Bool true)
  (BlockStmt)))");
    }

    SECTION("while flag { doWork() }") {
        auto fixture = createParserFixture("while flag { doWork() }");
        auto *stmt = fixture->parseStatement();



        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astIdentifier);
        REQUIRE(whileStmt->body != nullptr);
        REQUIRE(whileStmt->body->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (Identifier flag)
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier doWork))))))");
    }

    SECTION("while x > 0 { x = x - 1 }") {
        auto fixture = createParserFixture("while x > 0 { x = x - 1 }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astBinaryExpr);
        REQUIRE(whileStmt->body != nullptr);
        REQUIRE(whileStmt->body->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (BinaryExpr >
    (Identifier x)
    (Int 0))
  (BlockStmt
    (ExprStmt
      (AssignmentExpr =
        (Identifier x)
        (BinaryExpr -
          (Identifier x)
          (Int 1)))))))");
    }
}

TEST_CASE("While Statement Parsing - Parenthesized Expression Conditions", "[parser][statements][while-stmt][parens]") {
    SECTION("while (true) break") {
        auto fixture = createParserFixture("while (true) break");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astBool);
        REQUIRE(whileStmt->body != nullptr);
        REQUIRE(whileStmt->body->kind == astBreakStmt);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (Bool true)
  (BreakStmt)))");
    }

    SECTION("while (x > 0) x = x - 1") {
        auto fixture = createParserFixture("while (x > 0) x = x - 1");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astBinaryExpr);
        REQUIRE(whileStmt->body != nullptr);
        REQUIRE(whileStmt->body->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (BinaryExpr >
    (Identifier x)
    (Int 0))
  (ExprStmt
    (AssignmentExpr =
      (Identifier x)
      (BinaryExpr -
        (Identifier x)
        (Int 1))))))");
    }

    SECTION("while (ready && !done) { process() }") {
        auto fixture = createParserFixture("while (ready && !done) { process() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astBinaryExpr);
        REQUIRE(whileStmt->body != nullptr);
        REQUIRE(whileStmt->body->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (BinaryExpr &&
    (Identifier ready)
    (UnaryExpr !
      (Identifier done)))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier process))))))");
    }
}

TEST_CASE("While Statement Parsing - Variable Declaration Conditions", "[parser][statements][while-stmt][var-decl]") {
    SECTION("while const item = getNext() { process(item) }") {
        auto fixture = createParserFixture("while const item = getNext() { process(item) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astVariableDeclaration);
        REQUIRE(whileStmt->body != nullptr);
        REQUIRE(whileStmt->body->kind == astBlockStmt);

        auto *condDecl = static_cast<VariableDeclarationNode *>(whileStmt->condition);
        REQUIRE(condDecl->isConst());
        REQUIRE(condDecl->names.size() == 1);
        REQUIRE(condDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (VariableDeclaration
    (Identifier item)
    (CallExpr
      (Identifier getNext)))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier process)
        (Identifier item))))))");
    }

    SECTION("while var data = readData() { handleData(data) }") {
        auto fixture = createParserFixture("while var data = readData() { handleData(data) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astVariableDeclaration);

        auto *condDecl = static_cast<VariableDeclarationNode *>(whileStmt->condition);
        REQUIRE(!condDecl->isConst());
        REQUIRE(condDecl->names.size() == 1);
        REQUIRE(condDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (VariableDeclaration
    (Identifier data)
    (CallExpr
      (Identifier readData)))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier handleData)
        (Identifier data))))))");
    }

    SECTION("while auto line: string = readLine() { println(line) }") {
        auto fixture = createParserFixture("while auto line: string = readLine() { println(line) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astVariableDeclaration);

        auto *condDecl = static_cast<VariableDeclarationNode *>(whileStmt->condition);
        REQUIRE(!condDecl->isConst());
        REQUIRE(condDecl->names.size() == 1);
        REQUIRE(condDecl->type != nullptr);
        REQUIRE(condDecl->initializer != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (VariableDeclaration
    (Identifier line)
    (Type string)
    (CallExpr
      (Identifier readLine)))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier println)
        (Identifier line))))))");
    }
}

TEST_CASE("While Statement Parsing - Parenthesized Variable Declaration Conditions", "[parser][statements][while-stmt][parens][var-decl]") {
    SECTION("while (const line = readLine()) println(line)") {
        auto fixture = createParserFixture("while (const line = readLine()) println(line)");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astVariableDeclaration);
        REQUIRE(whileStmt->body != nullptr);
        REQUIRE(whileStmt->body->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (VariableDeclaration
    (Identifier line)
    (CallExpr
      (Identifier readLine)))
  (ExprStmt
    (CallExpr
      (Identifier println)
      (Identifier line)))))");
    }

    SECTION("while (var token = getToken()) processToken(token)") {
        auto fixture = createParserFixture("while (var token = getToken()) processToken(token)");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astVariableDeclaration);
        REQUIRE(whileStmt->body != nullptr);
        REQUIRE(whileStmt->body->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (VariableDeclaration
    (Identifier token)
    (CallExpr
      (Identifier getToken)))
  (ExprStmt
    (CallExpr
      (Identifier processToken)
      (Identifier token)))))");
    }

    SECTION("while (const value: i32 = getValue()) { work(value) }") {
        auto fixture = createParserFixture("while (const value: i32 = getValue()) { work(value) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astVariableDeclaration);
        REQUIRE(whileStmt->body != nullptr);
        REQUIRE(whileStmt->body->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (VariableDeclaration
    (Identifier value)
    (Type i32)
    (CallExpr
      (Identifier getValue)))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier work)
        (Identifier value))))))");
    }
}

TEST_CASE("While Statement Parsing - Error Cases", "[parser][statements][while-stmt][errors]") {
    SECTION("while without body") {
        auto fixture = createParserFixture("while true");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("while with multiple variable declaration (should fail)") {
        auto fixture = createParserFixture("while var a, b = getTuple() { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("while with variable declaration without initializer") {
        auto fixture = createParserFixture("while var x: i32 { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("while with malformed parentheses") {
        auto fixture = createParserFixture("while (true { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("while condition without parentheses - missing braces") {
        auto fixture = createParserFixture("while true println()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("while infinite loop without braces") {
        auto fixture = createParserFixture("while break");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("while with only opening brace") {
        auto fixture = createParserFixture("while true {");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("While Statement Parsing - Complex Conditions", "[parser][statements][while-stmt][complex]") {
    SECTION("while x > 0 && y < 10 || z == 5 { work() }") {
        auto fixture = createParserFixture("while x > 0 && y < 10 || z == 5 { work() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astBinaryExpr);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
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
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier work))))))");
    }

    SECTION("while hasNext() { processNext() }") {
        auto fixture = createParserFixture("while hasNext() { processNext() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astCallExpr);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (CallExpr
    (Identifier hasNext))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier processNext))))))");
    }

    SECTION("while !queue.isEmpty() { processItem(queue.dequeue()) }") {
        auto fixture = createParserFixture("while !queue.isEmpty() { processItem(queue.dequeue()) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astUnaryExpr);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (UnaryExpr !
    (CallExpr
      (MemberExpr .
        (Identifier queue)
        (Identifier isEmpty))))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier processItem)
        (CallExpr
          (MemberExpr .
            (Identifier queue)
            (Identifier dequeue))))))))");
    }
}

TEST_CASE("While Statement Parsing - Nested and Control Flow", "[parser][statements][while-stmt][nested]") {
    SECTION("while true { if ready { break } }") {
        auto fixture = createParserFixture("while true { if ready { break } }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->body != nullptr);
        REQUIRE(whileStmt->body->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (Bool true)
  (BlockStmt
    (IfStmt
      (Identifier ready)
      (BlockStmt
        (BreakStmt))))))");
    }

    SECTION("while const outer = getOuter() { while const inner = getInner(outer) { process(inner) } }") {
        auto fixture = createParserFixture("while const outer = getOuter() { while const inner = getInner(outer) { process(inner) } }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition != nullptr);
        REQUIRE(whileStmt->condition->kind == astVariableDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((WhileStmt
  (VariableDeclaration
    (Identifier outer)
    (CallExpr
      (Identifier getOuter)))
  (BlockStmt
    (WhileStmt
      (VariableDeclaration
        (Identifier inner)
        (CallExpr
          (Identifier getInner)
          (Identifier outer)))
      (BlockStmt
        (ExprStmt
          (CallExpr
            (Identifier process)
            (Identifier inner))))))))");
    }
}

TEST_CASE("While Statement Parsing - Statement Dispatch", "[parser][statements][while-stmt][dispatch]") {
    SECTION("parseStatement() correctly dispatches while statements") {
        auto fixture = createParserFixture("while true { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);
    }

    SECTION("while statement followed by other tokens") {
        auto fixture = createParserFixture("while true { } break");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        // Should be positioned at the next token after while statement
        REQUIRE(fixture->current().kind == TokenKind::Break);
    }

    SECTION("while infinite loop followed by other statements") {
        auto fixture = createParserFixture("while { } return");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astWhileStmt);

        auto *whileStmt = static_cast<WhileStatementNode *>(stmt);
        REQUIRE(whileStmt->condition == nullptr);

        // Should be positioned at the next token
        REQUIRE(fixture->current().kind == TokenKind::Return);
    }
}