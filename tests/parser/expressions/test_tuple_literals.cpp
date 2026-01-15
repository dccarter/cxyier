#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Simple tuple literals",
          "[parser][expressions][tuple][literals]") {
  SECTION("Two element tuple") {
    auto fixture = createParserFixture("(1, 2)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(TupleExpr (Int 1) (Int 2))");
  }

  SECTION("Three element tuple") {
    auto fixture = createParserFixture("(1, 2, 3)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(TupleExpr (Int 1) (Int 2) (Int 3))");
  }

  SECTION("Mixed type tuple") {
    auto fixture = createParserFixture("(42, x, 3.14)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(TupleExpr (Int 42) (Identifier x) (Float 3.14))");
  }
}

TEST_CASE("Parser: Tuple literals with expressions",
          "[parser][expressions][tuple][literals]") {
  SECTION("Tuple with arithmetic expressions") {
    auto fixture = createParserFixture("(1 + 2, 3 * 4)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (TupleExpr
        (BinaryExpr +
          (Int 1)
          (Int 2))
        (BinaryExpr *
          (Int 3)
          (Int 4)))
    )");
  }

  SECTION("Tuple with complex expressions") {
    auto fixture = createParserFixture("(a + b * c, x == y, true)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (TupleExpr
        (BinaryExpr +
          (Identifier a)
          (BinaryExpr *
            (Identifier b)
            (Identifier c)))
        (BinaryExpr ==
          (Identifier x)
          (Identifier y))
        (Bool true))
    )");
  }

  SECTION("Tuple with nested parentheses") {
    auto fixture = createParserFixture("((1 + 2), (x - y))");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (TupleExpr
        (BinaryExpr +
          (Int 1)
          (Int 2))
        (BinaryExpr -
          (Identifier x)
          (Identifier y)))
    )");
  }
}

TEST_CASE("Parser: Tuple literals with whitespace variations",
          "[parser][expressions][tuple][literals]") {
  SECTION("No spaces around commas") {
    auto fixture = createParserFixture("(1,2,3)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(TupleExpr (Int 1) (Int 2) (Int 3))");
  }

  SECTION("Extra spaces around commas") {
    auto fixture = createParserFixture("(1  ,  2  ,  3)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(TupleExpr (Int 1) (Int 2) (Int 3))");
  }

  SECTION("Newlines between elements") {
    auto fixture = createParserFixture("(\n  1,\n  2,\n  3\n)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(TupleExpr (Int 1) (Int 2) (Int 3))");
  }
}

TEST_CASE("Parser: Tuple vs parenthesized expression disambiguation",
          "[parser][expressions][tuple][literals][disambiguation]") {
  SECTION("Single expression in parentheses returns inner expression") {
    auto fixture = createParserFixture("(42)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(Int 42)");
  }

  SECTION("Single identifier in parentheses returns inner expression") {
    auto fixture = createParserFixture("(x)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(Identifier x)");
  }

  SECTION("Two elements is TupleExpr") {
    auto fixture = createParserFixture("(x, y)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(TupleExpr (Identifier x) (Identifier y))");
  }

  SECTION("Complex expression in parentheses returns inner expression") {
    auto fixture = createParserFixture("(1 + 2 * 3)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (Int 1)
        (BinaryExpr *
          (Int 2)
          (Int 3)))
    )");
  }
}

TEST_CASE("Parser: Tuple literals error cases",
          "[parser][expressions][tuple][literals][error]") {
  SECTION("Missing closing parenthesis") {
    auto fixture = createParserFixture("(1, 2, 3");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Missing comma between elements") {
    auto fixture = createParserFixture("(1 2 3)");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Trailing comma - should be allowed") {
    auto fixture = createParserFixture("(1, 2, 3,)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(TupleExpr (Int 1) (Int 2) (Int 3))");
  }

  SECTION("Double comma") {
    auto fixture = createParserFixture("(1,, 2)");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Empty tuple - not allowed") {
    auto fixture = createParserFixture("()");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }
}

TEST_CASE("Parser: Nested tuple literals",
          "[parser][expressions][tuple][literals][nested]") {
  SECTION("Tuple containing tuples") {
    auto fixture = createParserFixture("((1, 2), (3, 4))");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (TupleExpr
        (TupleExpr (Int 1) (Int 2))
        (TupleExpr (Int 3) (Int 4)))
    )");
  }

  SECTION("Mixed nesting with expressions") {
    auto fixture = createParserFixture("((1 + 2, 3), 4, (5, 6))");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (TupleExpr
        (TupleExpr
          (BinaryExpr +
            (Int 1)
            (Int 2))
          (Int 3))
        (Int 4)
        (TupleExpr (Int 5) (Int 6)))
    )");
  }

  SECTION("Tuple containing arrays") {
    auto fixture = createParserFixture("([1, 2], [3, 4])");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (TupleExpr
        (ArrayExpr (Int 1) (Int 2))
        (ArrayExpr (Int 3) (Int 4)))
    )");
  }
}
