#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Basic relational operations",
          "[parser][expressions][relational]") {
  SECTION("Simple less than") {
    auto fixture = createParserFixture("5 < 10");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr < (Int 5) (Int 10))");
  }

  SECTION("Simple less than or equal") {
    auto fixture = createParserFixture("3 <= 5");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr <= (Int 3) (Int 5))");
  }

  SECTION("Simple greater than") {
    auto fixture = createParserFixture("15 > 8");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr > (Int 15) (Int 8))");
  }

  SECTION("Simple greater than or equal") {
    auto fixture = createParserFixture("7 >= 7");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr >= (Int 7) (Int 7))");
  }

  SECTION("Relational with identifiers") {
    auto fixture = createParserFixture("x < y");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr < (Identifier x) (Identifier y))");
  }

  SECTION("Relational with mixed operands") {
    auto fixture = createParserFixture("count >= 100");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr >= (Identifier count) (Int 100))");
  }
}

TEST_CASE("Parser: Relational operator precedence",
          "[parser][expressions][relational][precedence]") {
  SECTION("Relational before shift") {
    auto fixture = createParserFixture("1 << 2 < 8");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <
        (BinaryExpr <<
          (Int 1)
          (Int 2))
        (Int 8))
    )");
  }

  SECTION("Relational before addition") {
    auto fixture = createParserFixture("5 + 3 > 7");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr >
        (BinaryExpr +
          (Int 5)
          (Int 3))
        (Int 7))
    )");
  }

  SECTION("Relational before multiplication") {
    auto fixture = createParserFixture("2 * 3 <= 6");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <=
        (BinaryExpr *
          (Int 2)
          (Int 3))
        (Int 6))
    )");
  }

  SECTION("Multiple precedence levels with relational") {
    auto fixture = createParserFixture("1 + 2 << 3 > 4");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr >
        (BinaryExpr <<
          (BinaryExpr +
            (Int 1)
            (Int 2))
          (Int 3))
        (Int 4))
    )");
  }
}

TEST_CASE("Parser: Relational left associativity",
          "[parser][expressions][relational][associativity]") {
  SECTION("Less than chain") {
    auto fixture = createParserFixture("1 < 2 < 3");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <
        (BinaryExpr <
          (Int 1)
          (Int 2))
        (Int 3))
    )");
  }

  SECTION("Greater than chain") {
    auto fixture = createParserFixture("10 > 5 > 2");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr >
        (BinaryExpr >
          (Int 10)
          (Int 5))
        (Int 2))
    )");
  }

  SECTION("Mixed relational operations") {
    auto fixture = createParserFixture("x <= y > z");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr >
        (BinaryExpr <=
          (Identifier x)
          (Identifier y))
        (Identifier z))
    )");
  }

  SECTION("All relational operators mixed") {
    auto fixture = createParserFixture("a < b <= c > d >= e");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr >=
        (BinaryExpr >
          (BinaryExpr <=
            (BinaryExpr <
              (Identifier a)
              (Identifier b))
            (Identifier c))
          (Identifier d))
        (Identifier e))
    )");
  }
}

TEST_CASE("Parser: Relational with parentheses",
          "[parser][expressions][relational][parentheses]") {
  SECTION("Override precedence with parentheses") {
    auto fixture = createParserFixture("(1 + 2) < 5");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <
        (BinaryExpr +
          (Int 1)
          (Int 2))
        (Int 5))
    )");
  }

  SECTION("Parentheses around relational operation") {
    auto fixture = createParserFixture("x + (y > z)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (Identifier x)
        (BinaryExpr >
          (Identifier y)
          (Identifier z)))
    )");
  }

  SECTION("Complex nested with parentheses") {
    auto fixture = createParserFixture("(a + b) >= (c * d)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr >=
        (BinaryExpr +
          (Identifier a)
          (Identifier b))
        (BinaryExpr *
          (Identifier c)
          (Identifier d)))
    )");
  }
}

TEST_CASE("Parser: Relational with whitespace",
          "[parser][expressions][relational]") {
  SECTION("Extra whitespace around operators") {
    auto fixture = createParserFixture("5   <=   10");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr <= (Int 5) (Int 10))");
  }

  SECTION("No whitespace around operators") {
    auto fixture = createParserFixture("x>y");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr > (Identifier x) (Identifier y))");
  }

  SECTION("Mixed whitespace") {
    auto fixture = createParserFixture(" 3+ 2 >= 5 ");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr >=
        (BinaryExpr +
          (Int 3)
          (Int 2))
        (Int 5))
    )");
  }
}

TEST_CASE("Parser: Relational with floats",
          "[parser][expressions][relational][types]") {
  SECTION("Float comparison") {
    auto fixture = createParserFixture("3.14 > 2.5");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr > (Float 3.14) (Float 2.5))");
  }

  SECTION("Mixed integer and float") {
    auto fixture = createParserFixture("10 <= 3.14159");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr <= (Int 10) (Float 3.14159))");
  }

  SECTION("Scientific notation") {
    auto fixture = createParserFixture("1e6 >= 1000000");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr >= (Float 1e+06) (Int 1000000))");
  }
}

TEST_CASE("Parser: Complex relational expressions",
          "[parser][expressions][relational][complex]") {
  SECTION("Relational with arithmetic chain") {
    auto fixture = createParserFixture("x * 2 + y / 3 < z - 1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <
        (BinaryExpr +
          (BinaryExpr *
            (Identifier x)
            (Int 2))
          (BinaryExpr /
            (Identifier y)
            (Int 3)))
        (BinaryExpr -
          (Identifier z)
          (Int 1)))
    )");
  }

  SECTION("Relational with shift operations") {
    auto fixture = createParserFixture("a << 2 > b >> 1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr >
        (BinaryExpr <<
          (Identifier a)
          (Int 2))
        (BinaryExpr >>
          (Identifier b)
          (Int 1)))
    )");
  }

  SECTION("Deeply nested expression") {
    auto fixture = createParserFixture("((a + b) * c) <= ((d - e) / f)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <=
        (BinaryExpr *
          (BinaryExpr +
            (Identifier a)
            (Identifier b))
          (Identifier c))
        (BinaryExpr /
          (BinaryExpr -
            (Identifier d)
            (Identifier e))
          (Identifier f)))
    )");
  }
}

TEST_CASE("Parser: Relational error cases",
          "[parser][expressions][relational][errors]") {
  SECTION("Missing operand after relational operator") {
    auto fixture = createParserFixture("5 < ");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Missing operand before relational operator") {
    auto fixture = createParserFixture("> 10");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Double relational operators") {
    auto fixture = createParserFixture("5 < < 10");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }
}

TEST_CASE("Parser: Relational expression sequence",
          "[parser][expressions][relational]") {
  SECTION("Relational expression followed by other tokens") {
    auto fixture = createParserFixture("x >= y ; a < b");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr >= (Identifier x) (Identifier y))");
    // Should stop at semicolon
    REQUIRE(fixture->current().kind == TokenKind::Semicolon);
  }
}
