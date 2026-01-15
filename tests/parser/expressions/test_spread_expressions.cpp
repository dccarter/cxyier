#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Basic spread expressions", "[parser][expressions][spread]") {
  SECTION("Simple spread expression") {
    auto fixture = createParserFixture("...array");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(SpreadExpr (Identifier array))");
  }

  SECTION("Spread with identifier") {
    auto fixture = createParserFixture("...items");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(SpreadExpr (Identifier items))");
  }

  SECTION("Spread with function call") {
    auto fixture = createParserFixture("...getTuple()");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(SpreadExpr (CallExpr (Identifier getTuple)))");
  }

  SECTION("Spread with member access") {
    auto fixture = createParserFixture("...obj.tuple");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr,
        "(SpreadExpr (MemberExpr . (Identifier obj) (Identifier tuple)))");
  }

  SECTION("Spread with indexing") {
    auto fixture = createParserFixture("...tuples[index]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr,
        "(SpreadExpr (IndexExpr (Identifier tuples) (Identifier index)))");
  }
}

TEST_CASE("Parser: Spread expressions in tuples",
          "[parser][expressions][spread]") {
  SECTION("Spread in tuple literal") {
    auto fixture = createParserFixture("(first, ...rest)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(TupleExpr (Identifier first) (SpreadExpr (Identifier rest)))");
  }

  SECTION("Multiple spreads in tuple") {
    auto fixture = createParserFixture("(...start, 42, ...end)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(TupleExpr (SpreadExpr (Identifier start)) (Int "
                              "42) (SpreadExpr (Identifier end)))");
  }

  SECTION("Spread only in tuple") {
    auto fixture = createParserFixture("(...items,)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(TupleExpr (SpreadExpr (Identifier items)))");
  }

  SECTION("Spread at beginning of tuple") {
    auto fixture = createParserFixture("(...prefix, a, b)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(TupleExpr (SpreadExpr (Identifier prefix)) "
                              "(Identifier a) (Identifier b))");
  }

  SECTION("Spread at end of tuple") {
    auto fixture = createParserFixture("(a, b, ...suffix)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(TupleExpr (Identifier a) (Identifier b) "
                              "(SpreadExpr (Identifier suffix)))");
  }

  SECTION("Spread in middle of tuple") {
    auto fixture = createParserFixture("(first, ...middle, last)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(TupleExpr (Identifier first) (SpreadExpr "
                              "(Identifier middle)) (Identifier last))");
  }
}

TEST_CASE("Parser: Spread expression precedence",
          "[parser][expressions][spread]") {
  SECTION("Spread has high precedence") {
    auto fixture = createParserFixture("...a + b");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(BinaryExpr + (SpreadExpr (Identifier a)) (Identifier b))");
  }

  SECTION("Spread with member access") {
    auto fixture = createParserFixture("...obj.method()");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(SpreadExpr (CallExpr (MemberExpr . (Identifier "
                              "obj) (Identifier method))))");
  }

  SECTION("Spread with complex expression") {
    auto fixture = createParserFixture("...getValue(x).tuple");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(SpreadExpr (MemberExpr . (CallExpr (Identifier "
                              "getValue) (Identifier x)) (Identifier tuple)))");
  }
}

TEST_CASE("Parser: Spread expression edge cases",
          "[parser][expressions][spread]") {
  SECTION("Spread with parenthesized expression") {
    auto fixture = createParserFixture("...(a + b)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(SpreadExpr (BinaryExpr + "
                              "(Identifier a) (Identifier b)))");
  }

  SECTION("Spread with nested tuple") {
    auto fixture = createParserFixture("...(1, 2, 3)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(SpreadExpr (TupleExpr (Int 1) (Int 2) (Int 3)))");
  }

  SECTION("Spread with tuple containing spread") {
    auto fixture = createParserFixture("(x, ...(a, ...b))");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(TupleExpr (Identifier x) (SpreadExpr (TupleExpr (Identifier a) "
              "(SpreadExpr (Identifier b)))))");
  }
}

TEST_CASE("Parser: Spread expression error cases",
          "[parser][expressions][spread]") {
  SECTION("Invalid spread without expression") {
    auto fixture = createParserFixture("...");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Spread with invalid expression") {
    auto fixture = createParserFixture("...+");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Double spread") {
    auto fixture = createParserFixture("......array");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Spread in single-element context (not a tuple)") {
    auto fixture = createParserFixture("(...items)");
    auto *expr = fixture->parseExpression();
    // This should parse as a spread expression in parentheses (not a tuple)
    REQUIRE_AST_MATCHES(expr, "(SpreadExpr (Identifier items))");
  }
}
