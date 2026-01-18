#include "catch2.hpp"
#include "../../parser_test_utils.hpp"
#include "cxy/ast/statements.hpp"
#include "cxy/ast/declarations.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/types.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::test;

TEST_CASE("Match Statement Parsing - Basic Type Patterns", "[parser][statements][match-stmt]") {
    SECTION("match x { i32 => println(\"integer\") }") {
        auto fixture = createParserFixture("match x { i32 => println(\"integer\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);

        auto *matchStmt = static_cast<MatchStatementNode *>(stmt);
        REQUIRE(matchStmt->discriminant != nullptr);
        REQUIRE(matchStmt->discriminant->kind == astIdentifier);
        REQUIRE(matchStmt->patterns.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((MatchStmt
  (Identifier x)
  (MatchCase
    (Type i32)
    (ExprStmt
      (CallExpr
        (Identifier println)
        (String "integer"))))))");
    }

    SECTION("match (value) { string => { println(\"text\") } }") {
        auto fixture = createParserFixture("match (value) { string => { println(\"text\") } }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);

        auto *matchStmt = static_cast<MatchStatementNode *>(stmt);
        REQUIRE(matchStmt->discriminant != nullptr);
        REQUIRE(matchStmt->patterns.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((MatchStmt
  (Identifier value)
  (MatchCase
    (Type string)
    (BlockStmt
      (ExprStmt
        (CallExpr
          (Identifier println)
          (String "text")))))))");
    }
}

TEST_CASE("Match Statement Parsing - Type Patterns with Variable Binding", "[parser][statements][match-stmt]") {
    SECTION("match x { i32 as a => println(a) }") {
        auto fixture = createParserFixture("match x { i32 as a => println(a) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);

        auto *matchStmt = static_cast<MatchStatementNode *>(stmt);
        REQUIRE(matchStmt->discriminant != nullptr);
        REQUIRE(matchStmt->patterns.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((MatchStmt
  (Identifier x)
  (MatchCase
    (Type i32)
    (Identifier a)
    (ExprStmt
      (CallExpr
        (Identifier println)
        (Identifier a))))))");
    }

    SECTION("match value { string as text => processText(text) }") {
        auto fixture = createParserFixture("match value { string as text => processText(text) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);

        auto *matchStmt = static_cast<MatchStatementNode *>(stmt);
        REQUIRE(matchStmt->patterns.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((MatchStmt
  (Identifier value)
  (MatchCase
    (Type string)
    (Identifier text)
    (ExprStmt
      (CallExpr
        (Identifier processText)
        (Identifier text))))))");
    }

    SECTION("match data { bool as flag => { handleFlag(flag) return flag } }") {
        auto fixture = createParserFixture("match data { bool as flag => { handleFlag(flag) return flag } }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);

        auto *matchStmt = static_cast<MatchStatementNode *>(stmt);
        REQUIRE(matchStmt->patterns.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((MatchStmt
  (Identifier data)
  (MatchCase
    (Type bool)
    (Identifier flag)
    (BlockStmt
      (ExprStmt
        (CallExpr
          (Identifier handleFlag)
          (Identifier flag)))
      (ReturnStmt
        (Identifier flag))))))");
    }
}

TEST_CASE("Match Statement Parsing - Multiple Types Per Case", "[parser][statements][match-stmt]") {
    SECTION("match value { i8, u8 => println(\"8-bit\") }") {
        auto fixture = createParserFixture("match value { i8, u8 => println(\"8-bit\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);

        auto *matchStmt = static_cast<MatchStatementNode *>(stmt);
        REQUIRE(matchStmt->patterns.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((MatchStmt
  (Identifier value)
  (MatchCase
    (Type i8)
    (Type u8)
    (ExprStmt
      (CallExpr
        (Identifier println)
        (String "8-bit"))))))");
    }

    SECTION("match x { i8, u8 as byte => processByte(byte) }") {
        auto fixture = createParserFixture("match x { i8, u8 as byte => processByte(byte) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);

        auto *matchStmt = static_cast<MatchStatementNode *>(stmt);
        REQUIRE(matchStmt->patterns.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((MatchStmt
  (Identifier x)
  (MatchCase
    (Type i8)
    (Type u8)
    (Identifier byte)
    (ExprStmt
      (CallExpr
        (Identifier processByte)
        (Identifier byte))))))");
    }

    SECTION("match input { i32, i64, f32, f64 as num => calculate(num) }") {
        auto fixture = createParserFixture("match input { i32, i64, f32, f64 as num => calculate(num) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);

        auto *matchStmt = static_cast<MatchStatementNode *>(stmt);
        REQUIRE(matchStmt->patterns.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((MatchStmt
  (Identifier input)
  (MatchCase
    (Type i32)
    (Type i64)
    (Type f32)
    (Type f64)
    (Identifier num)
    (ExprStmt
      (CallExpr
        (Identifier calculate)
        (Identifier num))))))");
    }
}

TEST_CASE("Match Statement Parsing - Default Cases", "[parser][statements][match-stmt]") {
    SECTION("match value { i32 => handleInt() ... => handleDefault() }") {
        auto fixture = createParserFixture("match value { i32 => handleInt() ... => handleDefault() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);

        auto *matchStmt = static_cast<MatchStatementNode *>(stmt);
        REQUIRE(matchStmt->patterns.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((MatchStmt
  (Identifier value)
  (MatchCase
    (Type i32)
    (ExprStmt
      (CallExpr
        (Identifier handleInt))))
  (MatchCase default
    (ExprStmt
      (CallExpr
        (Identifier handleDefault))))))");
    }

    SECTION("match obj { string as text => process(text) ... as other => handleOther(other) }") {
        auto fixture = createParserFixture("match obj { string as text => process(text) ... as other => handleOther(other) }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);

        auto *matchStmt = static_cast<MatchStatementNode *>(stmt);
        REQUIRE(matchStmt->patterns.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((MatchStmt
  (Identifier obj)
  (MatchCase
    (Type string)
    (Identifier text)
    (ExprStmt
      (CallExpr
        (Identifier process)
        (Identifier text))))
  (MatchCase default
    (Identifier other)
    (ExprStmt
      (CallExpr
        (Identifier handleOther)
        (Identifier other))))))");
    }

    SECTION("match data { ... => { println(\"unknown type\") return null } }") {
        auto fixture = createParserFixture("match data { ... => { println(\"unknown type\") return null } }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);

        auto *matchStmt = static_cast<MatchStatementNode *>(stmt);
        REQUIRE(matchStmt->patterns.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((MatchStmt
  (Identifier data)
  (MatchCase default
    (BlockStmt
      (ExprStmt
        (CallExpr
          (Identifier println)
          (String "unknown type")))
      (ReturnStmt
        (Null))))))");
    }
}

TEST_CASE("Match Statement Parsing - Complex Cases", "[parser][statements][match-stmt]") {
    SECTION("Multi-case match with mixed patterns") {
        auto fixture = createParserFixture(R"(
            match input {
                i32 as value => {
                    result = value * 2
                    println("Doubled: {result}")
                }
                string as text => {
                    upper = text.toUpperCase()
                    println("Upper: {upper}")
                }
                f32, f64 as float => processFloat(float)
                ... as other => {
                    println("Cannot process this type")
                    return null
                }
            }
        )");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);

        auto *matchStmt = static_cast<MatchStatementNode *>(stmt);
        REQUIRE(matchStmt->patterns.size() == 4);
    }

    SECTION("Trailing commas in type lists") {
        auto fixture = createParserFixture("match value { i8, u8, => handleBytes() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);

        auto *matchStmt = static_cast<MatchStatementNode *>(stmt);
        REQUIRE(matchStmt->patterns.size() == 1);
    }

    SECTION("Parenthesized discriminant with complex expression") {
        auto fixture = createParserFixture("match (getValue().getType()) { i32 => handle() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astMatchStmt);
    }
}

TEST_CASE("Match Statement Parsing - Error Cases", "[parser][statements][match-stmt][errors]") {
    SECTION("Missing arrow") {
        auto fixture = createParserFixture("match value { i32 println(\"integer\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing case body") {
        auto fixture = createParserFixture("match value { i32 => }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing braces") {
        auto fixture = createParserFixture("match value i32 => println(\"integer\")");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Empty case pattern") {
        auto fixture = createParserFixture("match value { => println(\"empty\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing discriminant") {
        auto fixture = createParserFixture("match { i32 => println(\"integer\") }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Invalid type in pattern") {
        auto fixture = createParserFixture("match value { 123invalid => handle() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Malformed type list") {
        auto fixture = createParserFixture("match value { i32,, u32 => handle() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing binding identifier after 'as'") {
        auto fixture = createParserFixture("match value { i32 as => handle() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Invalid binding identifier") {
        auto fixture = createParserFixture("match value { i32 as 123 => handle() }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing case body with default") {
        auto fixture = createParserFixture("match value { ... => }");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}