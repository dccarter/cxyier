#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Basic cast expressions", "[parser][expressions][cast]") {
  SECTION("Simple integer cast") {
    auto fixture = createParserFixture("x as i32");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(CastExpr as (Identifier x) (Type i32))");
  }

  SECTION("Float to integer cast") {
    auto fixture = createParserFixture("3.14 as i64");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(CastExpr as (Float 3.14) (Type i64))");
  }

  SECTION("String cast") {
    auto fixture = createParserFixture("value as string");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(CastExpr as (Identifier value) (Type string))");
  }

  SECTION("Auto cast") {
    auto fixture = createParserFixture("result as auto");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(CastExpr as (Identifier result) (Type auto))");
  }
}

TEST_CASE("Parser: Unsafe retype expressions",
          "[parser][expressions][cast][retype]") {
  SECTION("Simple unsafe retype") {
    auto fixture = createParserFixture("ptr !: u64");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(CastExpr !: (Identifier ptr) (Type u64))");
  }

  SECTION("Retype to string") {
    auto fixture = createParserFixture("data !: string");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(CastExpr !: (Identifier data) (Type string))");
  }
}

TEST_CASE("Parser: Cast expression precedence",
          "[parser][expressions][cast][precedence]") {
  SECTION("Cast has lower precedence than member access") {
    auto fixture = createParserFixture("obj.field as i32");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CastExpr as
        (MemberExpr .
          (Identifier obj)
          (Identifier field))
        (Type i32))
    )");
  }

  SECTION("Cast has higher precedence than addition") {
    auto fixture = createParserFixture("x as i32 + y");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (CastExpr as
          (Identifier x)
          (Type i32))
        (Identifier y))
    )");
  }
}

TEST_CASE("Parser: Chained cast expressions",
          "[parser][expressions][cast][chaining]") {
  SECTION("Multiple casts left-associative") {
    auto fixture = createParserFixture("x as i32 as f64");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CastExpr as
        (CastExpr as
          (Identifier x)
          (Type i32))
        (Type f64))
    )");
  }

  SECTION("Mixed cast and retype") {
    auto fixture = createParserFixture("data as u64 !: string");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CastExpr !:
        (CastExpr as
          (Identifier data)
          (Type u64))
        (Type string))
    )");
  }
}

TEST_CASE("Parser: Cast with complex expressions",
          "[parser][expressions][cast]") {
  SECTION("Cast parenthesized expression") {
    auto fixture = createParserFixture("(x + y) as i64");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CastExpr as
        (BinaryExpr +
          (Identifier x)
          (Identifier y))
        (Type i64))
    )");
  }

  SECTION("Retype with address-of") {
    auto fixture = createParserFixture("^variable !: u64");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, R"(
      (CastExpr !:
        (UnaryExpr ^
          (Identifier variable))
        (Type u64))
    )");
  }
}

TEST_CASE("Parser: Cast expression error cases",
          "[parser][expressions][cast][errors]") {
  SECTION("Missing type after as") {
    auto fixture = createParserFixture("x as");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Missing type after retype") {
    auto fixture = createParserFixture("ptr !:");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Invalid type syntax") {
    auto fixture = createParserFixture("x as 123invalid");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }
}

TEST_CASE("Parser: Cast with whitespace variations",
          "[parser][expressions][cast]") {
  SECTION("Extra whitespace around as") {
    auto fixture = createParserFixture("x   as   i32");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(CastExpr as (Identifier x) (Type i32))");
  }

  SECTION("Extra whitespace around retype") {
    auto fixture = createParserFixture("ptr   !:   string");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(CastExpr !: (Identifier ptr) (Type string))");
  }
}
