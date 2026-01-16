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

TEST_CASE("For Statement Parsing - Basic Single Variable", "[parser][statements][for-stmt]") {
    SECTION("for a in 0..10 { }") {
        auto fixture = createParserFixture("for a in 0..10 { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astForStmt);

        auto *forStmt = static_cast<ForStatementNode *>(stmt);
        REQUIRE(forStmt->variables.size() == 1);
        REQUIRE(forStmt->variables[0] != nullptr);
        REQUIRE(forStmt->variables[0]->kind == astIdentifier);
        REQUIRE(forStmt->range != nullptr);
        REQUIRE(forStmt->range->kind == astRangeExpr);
        REQUIRE(forStmt->condition == nullptr);
        REQUIRE(forStmt->body != nullptr);
        REQUIRE(forStmt->body->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((ForStmt
  (Variables a)
  (RangeExpr ..
    (Int 0)
    (Int 10))
  (BlockStmt)))");
    }

    SECTION("for item in collection { process(item) }") {
        auto fixture = createParserFixture("for item in collection { process(item) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astForStmt);

        auto *forStmt = static_cast<ForStatementNode *>(stmt);
        REQUIRE(forStmt->variables.size() == 1);
        REQUIRE(forStmt->range != nullptr);
        REQUIRE(forStmt->range->kind == astIdentifier);
        REQUIRE(forStmt->condition == nullptr);
        REQUIRE(forStmt->body != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((ForStmt
  (Variables item)
  (Identifier collection)
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier process)
        (Identifier item))))))");
    }
}

TEST_CASE("For Statement Parsing - Multiple Variables", "[parser][statements][for-stmt]") {
    SECTION("for a, b in pairs { }") {
        auto fixture = createParserFixture("for a, b in pairs { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astForStmt);

        auto *forStmt = static_cast<ForStatementNode *>(stmt);
        REQUIRE(forStmt->variables.size() == 2);
        REQUIRE(forStmt->variables[0] != nullptr);
        REQUIRE(forStmt->variables[1] != nullptr);
        REQUIRE(forStmt->range != nullptr);
        REQUIRE(forStmt->condition == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((ForStmt
  (Variables a b)
  (Identifier pairs)
  (BlockStmt)))");
    }

    SECTION("for value, idx in arr { println(value, idx) }") {
        auto fixture = createParserFixture("for value, idx in arr { println(value, idx) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astForStmt);

        auto *forStmt = static_cast<ForStatementNode *>(stmt);
        REQUIRE(forStmt->variables.size() == 2);
        REQUIRE(forStmt->range != nullptr);
        REQUIRE(forStmt->condition == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((ForStmt
  (Variables value idx)
  (Identifier arr)
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier println)
        (Identifier value)
        (Identifier idx))))))");
    }
}

TEST_CASE("For Statement Parsing - Wildcards", "[parser][statements][for-stmt]") {
    SECTION("for _ in items { processItem() }") {
        auto fixture = createParserFixture("for _ in items { processItem() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astForStmt);

        auto *forStmt = static_cast<ForStatementNode *>(stmt);
        REQUIRE(forStmt->variables.size() == 1);
        REQUIRE(forStmt->range != nullptr);
        REQUIRE(forStmt->condition == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((ForStmt
  (Variables _)
  (Identifier items)
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier processItem))))))");
    }

    SECTION("for value, _ in arr { process(value) }") {
        auto fixture = createParserFixture("for value, _ in arr { process(value) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astForStmt);

        auto *forStmt = static_cast<ForStatementNode *>(stmt);
        REQUIRE(forStmt->variables.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((ForStmt
  (Variables value _)
  (Identifier arr)
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier process)
        (Identifier value))))))");
    }
}

TEST_CASE("For Statement Parsing - With Condition", "[parser][statements][for-stmt]") {
    SECTION("for item in collection, item.isValid { use(item) }") {
        auto fixture = createParserFixture("for item in collection, item.isValid { use(item) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astForStmt);

        auto *forStmt = static_cast<ForStatementNode *>(stmt);
        REQUIRE(forStmt->variables.size() == 1);
        REQUIRE(forStmt->range != nullptr);
        REQUIRE(forStmt->condition != nullptr);
        REQUIRE(forStmt->condition->kind == astMemberExpr);

        REQUIRE_AST_MATCHES(stmt, R"((ForStmt
  (Variables item)
  (Identifier collection)
  (MemberExpr .
    (Identifier item)
    (Identifier isValid))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier use)
        (Identifier item))))))");
    }

    SECTION("for a, b in pairs, a > 0 { println(a, b) }") {
        auto fixture = createParserFixture("for a, b in pairs, a > 0 { println(a, b) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astForStmt);

        auto *forStmt = static_cast<ForStatementNode *>(stmt);
        REQUIRE(forStmt->variables.size() == 2);
        REQUIRE(forStmt->condition != nullptr);
        REQUIRE(forStmt->condition->kind == astBinaryExpr);

        REQUIRE_AST_MATCHES(stmt, R"((ForStmt
  (Variables a b)
  (Identifier pairs)
  (BinaryExpr >
    (Identifier a)
    (Int 0))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier println)
        (Identifier a)
        (Identifier b))))))");
    }
}

TEST_CASE("For Statement Parsing - Parenthesized Forms", "[parser][statements][for-stmt][parens]") {
    SECTION("for (a in 0..10) println(a)") {
        auto fixture = createParserFixture("for (a in 0..10) println(a)");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astForStmt);

        auto *forStmt = static_cast<ForStatementNode *>(stmt);
        REQUIRE(forStmt->variables.size() == 1);
        REQUIRE(forStmt->range != nullptr);
        REQUIRE(forStmt->condition == nullptr);
        REQUIRE(forStmt->body != nullptr);
        REQUIRE(forStmt->body->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, R"((ForStmt
  (Variables a)
  (RangeExpr ..
    (Int 0)
    (Int 10))
  (ExprStmt
    (CallExpr
      (Identifier println)
      (Identifier a)))))");
    }

    SECTION("for (a, b in pairs) print(a, b)") {
        auto fixture = createParserFixture("for (a, b in pairs) print(a, b)");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astForStmt);

        auto *forStmt = static_cast<ForStatementNode *>(stmt);
        REQUIRE(forStmt->variables.size() == 2);
        REQUIRE(forStmt->body->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, R"((ForStmt
  (Variables a b)
  (Identifier pairs)
  (ExprStmt
    (CallExpr
      (Identifier print)
      (Identifier a)
      (Identifier b)))))");
    }

    SECTION("for (item in collection, item.isValid) use(item)") {
        auto fixture = createParserFixture("for (item in collection, item.isValid) use(item)");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astForStmt);

        auto *forStmt = static_cast<ForStatementNode *>(stmt);
        REQUIRE(forStmt->variables.size() == 1);
        REQUIRE(forStmt->condition != nullptr);
        REQUIRE(forStmt->body->kind == astExprStmt);

        REQUIRE_AST_MATCHES(stmt, R"((ForStmt
  (Variables item)
  (Identifier collection)
  (MemberExpr .
    (Identifier item)
    (Identifier isValid))
  (ExprStmt
    (CallExpr
      (Identifier use)
      (Identifier item)))))");
    }

    SECTION("for (a in 0..5) { println(a) }") {
        auto fixture = createParserFixture("for (a in 0..5) { println(a) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astForStmt);

        auto *forStmt = static_cast<ForStatementNode *>(stmt);
        REQUIRE(forStmt->variables.size() == 1);
        REQUIRE(forStmt->body->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((ForStmt
  (Variables a)
  (RangeExpr ..
    (Int 0)
    (Int 5))
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier println)
        (Identifier a))))))");
    }
}

TEST_CASE("For Statement Parsing - Trailing Commas", "[parser][statements][for-stmt]") {
    SECTION("for value, idx, in arr { println(value, idx) }") {
        auto fixture = createParserFixture("for value, idx, in arr { println(value, idx) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astForStmt);

        auto *forStmt = static_cast<ForStatementNode *>(stmt);
        REQUIRE(forStmt->variables.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((ForStmt
  (Variables value idx)
  (Identifier arr)
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier println)
        (Identifier value)
        (Identifier idx))))))");
    }
}

TEST_CASE("For Statement Parsing - Error Cases", "[parser][statements][for-stmt][errors]") {
    SECTION("Missing 'in' keyword") {
        auto fixture = createParserFixture("for a 0..10 { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing range expression") {
        auto fixture = createParserFixture("for a in { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Empty variable list") {
        auto fixture = createParserFixture("for in range { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing body for bare form") {
        auto fixture = createParserFixture("for a in range");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Non-block body for bare form") {
        auto fixture = createParserFixture("for a in range println(a)");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Malformed variable list") {
        auto fixture = createParserFixture("for a,, b in range { }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}
