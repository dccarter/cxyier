#include "../../parser_test_utils.hpp"

#include "catch2.hpp"
#include <cmath>

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Basic float literals", "[parser][literals][float]") {
  SECTION("Simple float") {
    auto fixture = createParserFixture("3.14");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 3.14);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Zero float") {
    auto fixture = createParserFixture("0.0");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 0.0);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Float with leading zero") {
    auto fixture = createParserFixture("0.5");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 0.5);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Float with explicit leading zero") {
    auto fixture = createParserFixture("0.5");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 0.5);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Float without trailing fractional") {
    auto fixture = createParserFixture("42.");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 42.0);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Scientific notation floats", "[parser][literals][float]") {
  SECTION("Basic scientific notation") {
    auto fixture = createParserFixture("1e10");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 1e10);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Scientific notation with decimal") {
    auto fixture = createParserFixture("3.14e2");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 314.0);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Scientific notation with positive exponent") {
    auto fixture = createParserFixture("1.5e+3");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 1500.0);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Scientific notation with negative exponent") {
    auto fixture = createParserFixture("2.5e-2");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 0.025);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Uppercase E notation") {
    auto fixture = createParserFixture("1.23E4");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 12300.0);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Hexadecimal float literals", "[parser][literals][float]") {
  SECTION("Basic hex float") {
    auto fixture = createParserFixture("0x1.0p0");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 1.0);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Hex float with exponent") {
    auto fixture = createParserFixture("0x1.8p1");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 3.0); // 1.5 * 2^1
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Hex float with negative exponent") {
    auto fixture = createParserFixture("0x2.0p-1");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 1.0); // 2.0 * 2^-1
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Uppercase hex float") {
    auto fixture = createParserFixture("0X1.0P0");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 1.0);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Float literals with precision",
          "[parser][literals][float]") {
  SECTION("High precision float") {
    auto fixture = createParserFixture("3.141592653589793");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 3.141592653589793);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Small precision float") {
    auto fixture = createParserFixture("0.000001");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 0.000001);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Very small scientific") {
    auto fixture = createParserFixture("1.23e-10");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 1.23e-10);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Very large scientific") {
    auto fixture = createParserFixture("9.876e20");
    auto *node = fixture->parseLiteralExpression();
    expectFloatLiteral(node, 9.876e20);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Float literals in expressions",
          "[parser][literals][float]") {
  SECTION("Float in primary expression") {
    auto fixture = createParserFixture("2.718");
    auto *node = fixture->parsePrimaryExpression();
    expectFloatLiteral(node, 2.718);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Float in expression") {
    auto fixture = createParserFixture("1.414");
    auto *node = fixture->parseExpression();
    expectFloatLiteral(node, 1.414);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Multiple float literals", "[parser][literals][float]") {
  auto fixture = createParserFixture("1.1 2.2 3.3");

  // Parse first float
  auto *node1 = fixture->parseLiteralExpression();
  expectFloatLiteral(node1, 1.1);

  // Parse second float (parser advanced automatically)
  auto *node2 = fixture->parseLiteralExpression();
  expectFloatLiteral(node2, 2.2);

  // Parse third float (parser advanced automatically)
  auto *node3 = fixture->parseLiteralExpression();
  expectFloatLiteral(node3, 3.3);

  // Should be at end
  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Float special values", "[parser][literals][float]") {
  SECTION("Infinity") {
    auto fixture = createParserFixture("1e1000");
    auto *node = fixture->parseLiteralExpression();
    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astFloat);
    auto *floatNode = static_cast<ast::FloatLiteralNode *>(node);
    REQUIRE(std::isinf(floatNode->value));
  }

  SECTION("Very small number") {
    auto fixture = createParserFixture("1e-1000");
    auto *node = fixture->parseLiteralExpression();
    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astFloat);
    auto *floatNode = static_cast<ast::FloatLiteralNode *>(node);
    // Should be very close to zero
    REQUIRE(floatNode->value < 1e-100);
  }
}

TEST_CASE("Parser: Float literal error cases", "[parser][literals][float]") {
  SECTION("Wrong token type") {
    auto fixture = createParserFixture("\"not a float\"");
    auto *node = fixture->parseLiteralExpression();
    // Should parse as string, not float
    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astString);
  }

  SECTION("Empty input") {
    auto fixture = createParserFixture("");
    auto *node = fixture->parseLiteralExpression();
    expectParseFailure(node);
  }

  SECTION("Non-literal token") {
    auto fixture = createParserFixture("identifier");
    auto *node = fixture->parseLiteralExpression();
    expectParseFailure(node);
  }

  SECTION("Integer instead of float") {
    auto fixture = createParserFixture("42");
    auto *node = fixture->parseLiteralExpression();
    // Should parse as integer, not float
    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astInt);
  }
}

TEST_CASE("Parser: Float literal token buffer behavior",
          "[parser][literals][float]") {
  auto fixture = createParserFixture("3.14 42");

  // Initially at float
  REQUIRE(fixture->current().kind == TokenKind::FloatLiteral);
  REQUIRE(fixture->lookahead(1).kind == TokenKind::IntLiteral);

  // Parse float literal
  auto *node = fixture->parseLiteralExpression();
  expectFloatLiteral(node, 3.14);

  // Should have advanced to next token after parsing
  REQUIRE(fixture->current().kind == TokenKind::IntLiteral);
}

TEST_CASE("Parser: Float literal location information",
          "[parser][literals][float]") {
  auto fixture = createParserFixture("2.718281828");

  // Store the token location before parsing (parser will advance)
  Location expectedLocation = fixture->current().location;
  auto *node = fixture->parseLiteralExpression();

  REQUIRE(node != nullptr);
  REQUIRE(node->kind == ast::astFloat);

  // Location should be set
  REQUIRE(node->location.isValid());

  // Location should correspond to the original token
  REQUIRE(node->location == expectedLocation);
}

TEST_CASE("Parser: Float vs integer distinction", "[parser][literals][float]") {
  SECTION("Clear float") {
    auto fixture = createParserFixture("3.14");
    auto *node = fixture->parseLiteralExpression();
    REQUIRE(node->kind == ast::astFloat);
    expectFloatLiteral(node, 3.14);
  }

  SECTION("Clear integer") {
    auto fixture = createParserFixture("314");
    auto *node = fixture->parseLiteralExpression();
    REQUIRE(node->kind == ast::astInt);
    expectIntegerLiteral(node, 314);
  }

  SECTION("Scientific notation float") {
    auto fixture = createParserFixture("3e2");
    auto *node = fixture->parseLiteralExpression();
    REQUIRE(node->kind == ast::astFloat);
    expectFloatLiteral(node, 300.0);
  }

  SECTION("Decimal point float") {
    auto fixture = createParserFixture("3.");
    auto *node = fixture->parseLiteralExpression();
    REQUIRE(node->kind == ast::astFloat);
    expectFloatLiteral(node, 3.0);
  }
}

// Macro-based tests for consistency
LITERAL_TEST_CASE("Simple float", "3.14", expectFloatLiteral(node, 3.14))
LITERAL_TEST_CASE("Zero float", "0.0", expectFloatLiteral(node, 0.0))
LITERAL_TEST_CASE("Scientific notation", "1e10", expectFloatLiteral(node, 1e10))
LITERAL_TEST_CASE("Negative exponent", "2.5e-2",
                  expectFloatLiteral(node, 0.025))
LITERAL_TEST_CASE("Hex float", "0x1.0p0", expectFloatLiteral(node, 1.0))
LITERAL_TEST_CASE("Fractional with leading zero", "0.5",
                  expectFloatLiteral(node, 0.5))
LITERAL_TEST_CASE("Integer part only", "42.", expectFloatLiteral(node, 42.0))
