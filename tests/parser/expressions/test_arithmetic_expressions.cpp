#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Basic arithmetic operations",
          "[parser][expressions][arithmetic]") {
  SECTION("Simple addition") {
    auto fixture = createParserFixture("2 + 3");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr + (Int 2) (Int 3))");
  }

  SECTION("Simple subtraction") {
    auto fixture = createParserFixture("10 - 5");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr - (Int 10) (Int 5))");
  }

  SECTION("Simple multiplication") {
    auto fixture = createParserFixture("4 * 6");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr * (Int 4) (Int 6))");
  }

  SECTION("Simple division") {
    auto fixture = createParserFixture("8 / 2");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr / (Int 8) (Int 2))");
  }

  SECTION("Simple modulo") {
    auto fixture = createParserFixture("7 % 3");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr % (Int 7) (Int 3))");
  }
}

TEST_CASE("Parser: Arithmetic with identifiers and mixed types",
          "[parser][expressions][arithmetic]") {
  SECTION("Addition with identifiers") {
    auto fixture = createParserFixture("x + y");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr + (Identifier x) (Identifier y))");
  }

  SECTION("Multiplication with identifier and literal") {
    auto fixture = createParserFixture("variable * 5");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr * (Identifier variable) (Int 5))");
  }

  SECTION("Mixed with floats") {
    auto fixture = createParserFixture("3.14 + 2.0");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr + (Float 3.14) (Float 2))");
  }

  SECTION("Mixed integer and float") {
    auto fixture = createParserFixture("42 - 3.14");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr - (Int 42) (Float 3.14))");
  }
}

TEST_CASE("Parser: Operator precedence",
          "[parser][expressions][arithmetic][precedence]") {
  SECTION("Multiplication before addition") {
    auto fixture = createParserFixture("2 + 3 * 4");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (Int 2)
        (BinaryExpr *
          (Int 3)
          (Int 4)))
    )");
  }

  SECTION("Division before subtraction") {
    auto fixture = createParserFixture("10 - 8 / 2");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr -
        (Int 10)
        (BinaryExpr /
          (Int 8)
          (Int 2)))
    )");
  }

  SECTION("Modulo before addition") {
    auto fixture = createParserFixture("5 + 7 % 3");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (Int 5)
        (BinaryExpr %
          (Int 7)
          (Int 3)))
    )");
  }

  SECTION("Multiple precedence levels") {
    auto fixture = createParserFixture("1 + 2 * 3 - 4 / 2");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr -
        (BinaryExpr +
          (Int 1)
          (BinaryExpr *
            (Int 2)
            (Int 3)))
        (BinaryExpr /
          (Int 4)
          (Int 2)))
    )");
  }
}

TEST_CASE("Parser: Left associativity",
          "[parser][expressions][arithmetic][associativity]") {
  SECTION("Addition chain") {
    auto fixture = createParserFixture("1 + 2 + 3");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (BinaryExpr +
          (Int 1)
          (Int 2))
        (Int 3))
    )");
  }

  SECTION("Subtraction chain") {
    auto fixture = createParserFixture("10 - 5 - 2");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr -
        (BinaryExpr -
          (Int 10)
          (Int 5))
        (Int 2))
    )");
  }

  SECTION("Multiplication chain") {
    auto fixture = createParserFixture("2 * 3 * 4");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr *
        (BinaryExpr *
          (Int 2)
          (Int 3))
        (Int 4))
    )");
  }

  SECTION("Division chain") {
    auto fixture = createParserFixture("16 / 4 / 2");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr /
        (BinaryExpr /
          (Int 16)
          (Int 4))
        (Int 2))
    )");
  }

  SECTION("Mixed same-precedence operations") {
    auto fixture = createParserFixture("20 / 4 * 3");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr *
        (BinaryExpr /
          (Int 20)
          (Int 4))
        (Int 3))
    )");
  }
}

TEST_CASE("Parser: Parenthesized arithmetic expressions",
          "[parser][expressions][arithmetic][parentheses]") {
  SECTION("Override precedence with parentheses") {
    auto fixture = createParserFixture("(2 + 3) * 4");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr *
        (BinaryExpr +
          (Int 2)
          (Int 3))
        (Int 4))
    )");
  }

  SECTION("Multiple parentheses") {
    auto fixture = createParserFixture("(10 - 6) / (2 + 2)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr /
        (BinaryExpr -
          (Int 10)
          (Int 6))
        (BinaryExpr +
          (Int 2)
          (Int 2)))
    )");
  }

  SECTION("Nested parentheses") {
    auto fixture = createParserFixture("((2 + 3) * 4)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr *
        (BinaryExpr +
          (Int 2)
          (Int 3))
        (Int 4))
    )");
  }

  SECTION("Complex nested expression") {
    auto fixture = createParserFixture("(1 + 2) * (3 - 4) / (5 + 6)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr /
        (BinaryExpr *
          (BinaryExpr +
            (Int 1)
            (Int 2))
          (BinaryExpr -
            (Int 3)
            (Int 4)))
        (BinaryExpr +
          (Int 5)
          (Int 6)))
    )");
  }
}

TEST_CASE("Parser: Arithmetic with whitespace",
          "[parser][expressions][arithmetic]") {
  SECTION("Extra whitespace around operators") {
    auto fixture = createParserFixture("2   +   3");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr + (Int 2) (Int 3))");
  }

  SECTION("No whitespace around operators") {
    auto fixture = createParserFixture("4*5");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr * (Int 4) (Int 5))");
  }

  SECTION("Mixed whitespace") {
    auto fixture = createParserFixture(" 10- 6 /2 ");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr -
        (Int 10)
        (BinaryExpr /
          (Int 6)
          (Int 2)))
    )");
  }

  SECTION("Newlines in expressions") {
    auto fixture = createParserFixture("1 +\n2 *\n3");
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

TEST_CASE("Parser: Complex arithmetic expressions",
          "[parser][expressions][arithmetic][complex]") {
  SECTION("Long expression chain") {
    auto fixture = createParserFixture("1 + 2 - 3 + 4 - 5");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr -
        (BinaryExpr +
          (BinaryExpr -
            (BinaryExpr +
              (Int 1)
              (Int 2))
            (Int 3))
          (Int 4))
        (Int 5))
    )");
  }

  SECTION("Mixed operators with identifiers") {
    auto fixture = createParserFixture("x * 2 + y / 3 - z % 4");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr -
        (BinaryExpr +
          (BinaryExpr *
            (Identifier x)
            (Int 2))
          (BinaryExpr /
            (Identifier y)
            (Int 3)))
        (BinaryExpr %
          (Identifier z)
          (Int 4)))
    )");
  }

  SECTION("Deeply nested with parentheses") {
    auto fixture =
        createParserFixture("((a + b) * (c - d)) / ((e + f) - (g * h))");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr /
        (BinaryExpr *
          (BinaryExpr +
            (Identifier a)
            (Identifier b))
          (BinaryExpr -
            (Identifier c)
            (Identifier d)))
        (BinaryExpr -
          (BinaryExpr +
            (Identifier e)
            (Identifier f))
          (BinaryExpr *
            (Identifier g)
            (Identifier h))))
    )");
  }
}

TEST_CASE("Parser: Arithmetic error cases",
          "[parser][expressions][arithmetic][errors]") {
  SECTION("Missing operand after operator") {
    auto fixture = createParserFixture("5 + ");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Unary plus operator") {
    auto fixture = createParserFixture("+ 5");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr != nullptr);
    REQUIRE_AST_MATCHES(expr, "(UnaryExpr + (Int 5))");
  }

  SECTION("Address-of operator") {
    auto fixture = createParserFixture("^variable");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr != nullptr);
    REQUIRE_AST_MATCHES(expr, "(UnaryExpr ^ (Identifier variable))");
  }

  SECTION("Binary plus with unary plus") {
    auto fixture = createParserFixture("5 + + 3");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr != nullptr);
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr + (Int 5) (UnaryExpr + (Int 3)))");
  }

  SECTION("Missing closing parenthesis") {
    auto fixture = createParserFixture("(2 + 3 * 4");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Missing opening parenthesis") {
    auto fixture = createParserFixture("2 + 3) * 4");
    auto *expr = fixture->parseExpression();
    // Should parse "2 + 3" successfully, leaving ") * 4" as remaining tokens
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr + (Int 2) (Int 3))");
    REQUIRE(fixture->current().kind == TokenKind::RParen);
  }

  SECTION("Empty parentheses") {
    auto fixture = createParserFixture("5 + ()");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }
}

TEST_CASE("Parser: Arithmetic with different number types",
          "[parser][expressions][arithmetic][types]") {
  SECTION("Large integers") {
    auto fixture = createParserFixture("999999999 + 1000000000");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(BinaryExpr + (Int 999999999) (Int 1000000000))");
  }

  SECTION("Negative numbers as literals") {
    // Note: This tests if negative numbers are parsed as unary minus + positive
    // number or as negative literals directly from the lexer
    auto fixture = createParserFixture("5 + -3");
    auto *expr = fixture->parseExpression();
    // This will depend on how the lexer handles negative numbers
    // For now, assume unary minus
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (Int 5)
        (UnaryExpr -
          (Int 3)))
    )");
  }

  SECTION("Float precision") {
    auto fixture = createParserFixture("0.1 + 0.2");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr + (Float 0.1) (Float 0.2))");
  }

  SECTION("Scientific notation") {
    auto fixture = createParserFixture("1e6 * 2e-3");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr * (Float 1e+06) (Float 0.002))");
  }
}

TEST_CASE("Parser: Arithmetic expression sequence",
          "[parser][expressions][arithmetic]") {
  SECTION("Multiple expressions separated by end-of-statement") {
    auto fixture = createParserFixture("1 + 2");
    auto *expr1 = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr1, "(BinaryExpr + (Int 1) (Int 2))");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Expression followed by other tokens") {
    auto fixture = createParserFixture("3 * 4 ; 5 + 6");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr * (Int 3) (Int 4))");
    // Should stop at semicolon
    REQUIRE(fixture->current().kind == TokenKind::Semicolon);
  }
}
