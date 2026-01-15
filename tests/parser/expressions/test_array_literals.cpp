#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Empty array literals",
          "[parser][expressions][array][literals]") {
  SECTION("Empty array") {
    auto fixture = createParserFixture("[]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(ArrayExpr)");
  }

  SECTION("Empty array with spaces") {
    auto fixture = createParserFixture("[ ]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(ArrayExpr)");
  }
}

TEST_CASE("Parser: Simple array literals",
          "[parser][expressions][array][literals]") {
  SECTION("Single integer element") {
    auto fixture = createParserFixture("[42]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(ArrayExpr (Int 42))");
  }

  SECTION("Single identifier element") {
    auto fixture = createParserFixture("[x]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(ArrayExpr (Identifier x))");
  }

  SECTION("Multiple integer elements") {
    auto fixture = createParserFixture("[1, 2, 3]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(ArrayExpr (Int 1) (Int 2) (Int 3))");
  }

  SECTION("Multiple mixed elements") {
    auto fixture = createParserFixture("[42, x, 3.14]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(ArrayExpr (Int 42) (Identifier x) (Float 3.14))");
  }
}

TEST_CASE("Parser: Array literals with expressions",
          "[parser][expressions][array][literals]") {
  SECTION("Array with arithmetic expressions") {
    auto fixture = createParserFixture("[1 + 2, 3 * 4]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (ArrayExpr
        (BinaryExpr +
          (Int 1)
          (Int 2))
        (BinaryExpr *
          (Int 3)
          (Int 4)))
    )");
  }

  SECTION("Array with nested parentheses") {
    auto fixture = createParserFixture("[(1 + 2), (x - y)]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (ArrayExpr
        (BinaryExpr +
          (Int 1)
          (Int 2))
        (BinaryExpr -
          (Identifier x)
          (Identifier y)))
    )");
  }

  SECTION("Array with complex expressions") {
    auto fixture = createParserFixture("[a + b * c, x == y]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (ArrayExpr
        (BinaryExpr +
          (Identifier a)
          (BinaryExpr *
            (Identifier b)
            (Identifier c)))
        (BinaryExpr ==
          (Identifier x)
          (Identifier y)))
    )");
  }
}

TEST_CASE("Parser: Array literals with whitespace variations",
          "[parser][expressions][array][literals]") {
  SECTION("No spaces around commas") {
    auto fixture = createParserFixture("[1,2,3]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(ArrayExpr (Int 1) (Int 2) (Int 3))");
  }

  SECTION("Extra spaces around commas") {
    auto fixture = createParserFixture("[1  ,  2  ,  3]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(ArrayExpr (Int 1) (Int 2) (Int 3))");
  }

  SECTION("Newlines between elements") {
    auto fixture = createParserFixture("[\n  1,\n  2,\n  3\n]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(ArrayExpr (Int 1) (Int 2) (Int 3))");
  }
}

TEST_CASE("Parser: Array literals error cases",
          "[parser][expressions][array][literals][error]") {
  SECTION("Missing closing bracket") {
    auto fixture = createParserFixture("[1, 2, 3");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Missing comma between elements") {
    auto fixture = createParserFixture("[1 2 3]");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Trailing comma - should be allowed") {
    auto fixture = createParserFixture("[1, 2, 3,]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(ArrayExpr (Int 1) (Int 2) (Int 3))");
  }

  SECTION("Double comma") {
    auto fixture = createParserFixture("[1,, 2]");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }
}

TEST_CASE("Parser: Nested array literals",
          "[parser][expressions][array][literals][nested]") {
  SECTION("Simple nested arrays") {
    auto fixture = createParserFixture("[[1, 2], [3, 4]]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (ArrayExpr
        (ArrayExpr (Int 1) (Int 2))
        (ArrayExpr (Int 3) (Int 4)))
    )");
  }

  SECTION("Deeply nested arrays") {
    auto fixture = createParserFixture("[[[1]]]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (ArrayExpr
        (ArrayExpr
          (ArrayExpr (Int 1))))
    )");
  }

  SECTION("Mixed nesting with expressions") {
    auto fixture = createParserFixture("[[1 + 2], 3, [4, 5]]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (ArrayExpr
        (ArrayExpr
          (BinaryExpr +
            (Int 1)
            (Int 2)))
        (Int 3)
        (ArrayExpr (Int 4) (Int 5)))
    )");
  }
}
