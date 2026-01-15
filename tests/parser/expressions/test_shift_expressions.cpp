#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Basic shift operations", "[parser][expressions][shift]") {
  SECTION("Simple left shift") {
    auto fixture = createParserFixture("4 << 2");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr << (Int 4) (Int 2))");
  }

  SECTION("Simple right shift") {
    auto fixture = createParserFixture("16 >> 3");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr >> (Int 16) (Int 3))");
  }

  SECTION("Left shift with identifiers") {
    auto fixture = createParserFixture("value << count");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(BinaryExpr << (Identifier value) (Identifier count))");
  }

  SECTION("Right shift with mixed operands") {
    auto fixture = createParserFixture("x >> 4");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr >> (Identifier x) (Int 4))");
  }
}

TEST_CASE("Parser: Shift operator precedence",
          "[parser][expressions][shift][precedence]") {
  SECTION("Shift before addition") {
    auto fixture = createParserFixture("1 + 2 << 3");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <<
        (BinaryExpr +
          (Int 1)
          (Int 2))
        (Int 3))
    )");
  }

  SECTION("Shift before subtraction") {
    auto fixture = createParserFixture("8 - 1 >> 2");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr >>
        (BinaryExpr -
          (Int 8)
          (Int 1))
        (Int 2))
    )");
  }

  SECTION("Shift before multiplication") {
    auto fixture = createParserFixture("2 * 3 << 1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <<
        (BinaryExpr *
          (Int 2)
          (Int 3))
        (Int 1))
    )");
  }

  SECTION("Multiple precedence levels with shift") {
    auto fixture = createParserFixture("1 + 2 * 3 << 4");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <<
        (BinaryExpr +
          (Int 1)
          (BinaryExpr *
            (Int 2)
            (Int 3)))
        (Int 4))
    )");
  }
}

TEST_CASE("Parser: Shift left associativity",
          "[parser][expressions][shift][associativity]") {
  SECTION("Left shift chain") {
    auto fixture = createParserFixture("1 << 2 << 3");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <<
        (BinaryExpr <<
          (Int 1)
          (Int 2))
        (Int 3))
    )");
  }

  SECTION("Right shift chain") {
    auto fixture = createParserFixture("64 >> 2 >> 1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr >>
        (BinaryExpr >>
          (Int 64)
          (Int 2))
        (Int 1))
    )");
  }

  SECTION("Mixed shift operations") {
    auto fixture = createParserFixture("16 << 1 >> 2");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr >>
        (BinaryExpr <<
          (Int 16)
          (Int 1))
        (Int 2))
    )");
  }
}

TEST_CASE("Parser: Shift with parentheses",
          "[parser][expressions][shift][parentheses]") {
  SECTION("Override precedence with parentheses") {
    auto fixture = createParserFixture("(1 + 2) << 3");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <<
        (BinaryExpr +
          (Int 1)
          (Int 2))
        (Int 3))
    )");
  }

  SECTION("Parentheses around shift operation") {
    auto fixture = createParserFixture("4 + (2 << 3)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (Int 4)
        (BinaryExpr <<
          (Int 2)
          (Int 3)))
    )");
  }

  SECTION("Complex nested with parentheses") {
    auto fixture = createParserFixture("(a + b) << (c - d)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <<
        (BinaryExpr +
          (Identifier a)
          (Identifier b))
        (BinaryExpr -
          (Identifier c)
          (Identifier d)))
    )");
  }
}

TEST_CASE("Parser: Shift with whitespace", "[parser][expressions][shift]") {
  SECTION("Extra whitespace around operators") {
    auto fixture = createParserFixture("4   <<   2");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr << (Int 4) (Int 2))");
  }

  SECTION("No whitespace around operators") {
    auto fixture = createParserFixture("8>>1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr >> (Int 8) (Int 1))");
  }

  SECTION("Mixed whitespace") {
    auto fixture = createParserFixture(" 16 << 2+ 1 ");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <<
        (Int 16)
        (BinaryExpr +
          (Int 2)
          (Int 1)))
    )");
  }
}

TEST_CASE("Parser: Complex shift expressions",
          "[parser][expressions][shift][complex]") {
  SECTION("Shift with arithmetic chain") {
    auto fixture = createParserFixture("1 + 2 - 3 << 4 >> 1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr >>
        (BinaryExpr <<
          (BinaryExpr -
            (BinaryExpr +
              (Int 1)
              (Int 2))
            (Int 3))
          (Int 4))
        (Int 1))
    )");
  }

  SECTION("Mixed operators with identifiers") {
    auto fixture = createParserFixture("x * 2 + y << z");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr <<
        (BinaryExpr +
          (BinaryExpr *
            (Identifier x)
            (Int 2))
          (Identifier y))
        (Identifier z))
    )");
  }
}

TEST_CASE("Parser: Shift error cases", "[parser][expressions][shift][errors]") {
  SECTION("Missing operand after shift operator") {
    auto fixture = createParserFixture("5 << ");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Missing operand before shift operator") {
    auto fixture = createParserFixture("<< 3");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Double shift operators") {
    auto fixture = createParserFixture("5 << << 3");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }
}

TEST_CASE("Parser: Shift expression sequence", "[parser][expressions][shift]") {
  SECTION("Shift expression followed by other tokens") {
    auto fixture = createParserFixture("4 << 2 ; 8 >> 1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(BinaryExpr << (Int 4) (Int 2))");
    // Should stop at semicolon
    REQUIRE(fixture->current().kind == TokenKind::Semicolon);
  }
}
