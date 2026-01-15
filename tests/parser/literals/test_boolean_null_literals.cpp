#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Boolean literals", "[parser][literals][boolean]") {
  SECTION("True literal") {
    auto fixture = createParserFixture("true");
    auto *node = fixture->parseLiteralExpression();
    expectBoolLiteral(node, true);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("False literal") {
    auto fixture = createParserFixture("false");
    auto *node = fixture->parseLiteralExpression();
    expectBoolLiteral(node, false);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Boolean literals in expressions",
          "[parser][literals][boolean]") {
  SECTION("True in primary expression") {
    auto fixture = createParserFixture("true");
    auto *node = fixture->parsePrimaryExpression();
    expectBoolLiteral(node, true);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("False in primary expression") {
    auto fixture = createParserFixture("false");
    auto *node = fixture->parsePrimaryExpression();
    expectBoolLiteral(node, false);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("True in expression") {
    auto fixture = createParserFixture("true");
    auto *node = fixture->parseExpression();
    expectBoolLiteral(node, true);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("False in expression") {
    auto fixture = createParserFixture("false");
    auto *node = fixture->parseExpression();
    expectBoolLiteral(node, false);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Multiple boolean literals", "[parser][literals][boolean]") {
  auto fixture = createParserFixture("true false true");

  // Parse first boolean
  auto *node1 = fixture->parseLiteralExpression();
  expectBoolLiteral(node1, true);

  // Parse second boolean (parser advanced automatically)
  auto *node2 = fixture->parseLiteralExpression();
  expectBoolLiteral(node2, false);

  // Parse third boolean (parser advanced automatically)
  auto *node3 = fixture->parseLiteralExpression();
  expectBoolLiteral(node3, true);

  // Should be at end
  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Boolean literal token buffer behavior",
          "[parser][literals][boolean]") {
  auto fixture = createParserFixture("true false");

  // Initially at true
  REQUIRE(fixture->current().kind == TokenKind::True);
  REQUIRE(fixture->lookahead(1).kind == TokenKind::False);

  // Parse boolean literal
  auto *node = fixture->parseLiteralExpression();
  expectBoolLiteral(node, true);

  // Should have advanced to next token after parsing
  REQUIRE(fixture->current().kind == TokenKind::False);
}

TEST_CASE("Parser: Boolean literal location information",
          "[parser][literals][boolean]") {
  SECTION("True location") {
    auto fixture = createParserFixture("true");

    // Store the token location before parsing (parser will advance)
    Location expectedLocation = fixture->current().location;
    auto *node = fixture->parseLiteralExpression();

    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astBool);

    // Location should be set
    REQUIRE(node->location.isValid());

    // Location should correspond to the original token
    REQUIRE(node->location == expectedLocation);
  }

  SECTION("False location") {
    auto fixture = createParserFixture("false");

    // Store the token location before parsing (parser will advance)
    Location expectedLocation = fixture->current().location;
    auto *node = fixture->parseLiteralExpression();

    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astBool);

    // Location should be set
    REQUIRE(node->location.isValid());

    // Location should correspond to the original token
    REQUIRE(node->location == expectedLocation);
  }
}

TEST_CASE("Parser: Boolean literal error cases",
          "[parser][literals][boolean]") {
  SECTION("Wrong token type") {
    auto fixture = createParserFixture("42");
    auto *node = fixture->parseLiteralExpression();
    // Should parse as integer, not boolean
    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astInt);
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

  SECTION("String instead of boolean") {
    auto fixture = createParserFixture("\"true\"");
    auto *node = fixture->parseLiteralExpression();
    // Should parse as string, not boolean
    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astString);
  }
}

// Null literal tests

TEST_CASE("Parser: Null literal", "[parser][literals][null]") {
  SECTION("Basic null") {
    auto fixture = createParserFixture("null");
    auto *node = fixture->parseLiteralExpression();
    expectNullLiteral(node);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Null literal in expressions", "[parser][literals][null]") {
  SECTION("Null in primary expression") {
    auto fixture = createParserFixture("null");
    auto *node = fixture->parsePrimaryExpression();
    expectNullLiteral(node);
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Null in expression") {
    auto fixture = createParserFixture("null");
    auto *node = fixture->parseExpression();
    expectNullLiteral(node);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Multiple null literals", "[parser][literals][null]") {
  auto fixture = createParserFixture("null null null");

  // Parse first null
  auto *node1 = fixture->parseLiteralExpression();
  expectNullLiteral(node1);

  // Parse second null (parser advanced automatically)
  auto *node2 = fixture->parseLiteralExpression();
  expectNullLiteral(node2);

  // Parse third null (parser advanced automatically)
  auto *node3 = fixture->parseLiteralExpression();
  expectNullLiteral(node3);

  // Should be at end
  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Null literal token buffer behavior",
          "[parser][literals][null]") {
  auto fixture = createParserFixture("null true");

  // Initially at null
  REQUIRE(fixture->current().kind == TokenKind::Null);
  REQUIRE(fixture->lookahead(1).kind == TokenKind::True);

  // Parse null literal
  auto *node = fixture->parseLiteralExpression();
  expectNullLiteral(node);

  // Should have advanced to next token after parsing
  REQUIRE(fixture->current().kind == TokenKind::True);
}

TEST_CASE("Parser: Null literal location information",
          "[parser][literals][null]") {
  auto fixture = createParserFixture("null");

  // Store the token location before parsing (parser will advance)
  Location expectedLocation = fixture->current().location;
  auto *node = fixture->parseLiteralExpression();

  REQUIRE(node != nullptr);
  REQUIRE(node->kind == ast::astNull);

  // Location should be set
  REQUIRE(node->location.isValid());

  // Location should correspond to the original token
  REQUIRE(node->location == expectedLocation);
}

TEST_CASE("Parser: Null literal error cases", "[parser][literals][null]") {
  SECTION("Wrong token type") {
    auto fixture = createParserFixture("42");
    auto *node = fixture->parseLiteralExpression();
    // Should parse as integer, not null
    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astInt);
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

  SECTION("String instead of null") {
    auto fixture = createParserFixture("\"null\"");
    auto *node = fixture->parseLiteralExpression();
    // Should parse as string, not null
    REQUIRE(node != nullptr);
    REQUIRE(node->kind == ast::astString);
  }
}

// Mixed boolean and null tests

TEST_CASE("Parser: Mixed boolean and null literals",
          "[parser][literals][boolean][null]") {
  auto fixture = createParserFixture("true null false null");

  // Parse sequence
  auto *node1 = fixture->parseLiteralExpression();
  expectBoolLiteral(node1, true);

  auto *node2 = fixture->parseLiteralExpression();
  expectNullLiteral(node2);

  auto *node3 = fixture->parseLiteralExpression();
  expectBoolLiteral(node3, false);

  auto *node4 = fixture->parseLiteralExpression();
  expectNullLiteral(node4);

  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Boolean and null with other literals",
          "[parser][literals][boolean][null]") {
  auto fixture = createParserFixture("42 true \"hello\" null 3.14 false");

  // Test mixed sequence
  auto *node1 = fixture->parseLiteralExpression();
  expectIntegerLiteral(node1, 42);

  auto *node2 = fixture->parseLiteralExpression();
  expectBoolLiteral(node2, true);

  auto *node3 = fixture->parseLiteralExpression();
  expectStringLiteral(node3, "hello");

  auto *node4 = fixture->parseLiteralExpression();
  expectNullLiteral(node4);

  auto *node5 = fixture->parseLiteralExpression();
  expectFloatLiteral(node5, 3.14);

  auto *node6 = fixture->parseLiteralExpression();
  expectBoolLiteral(node6, false);

  REQUIRE(fixture->isAtEnd());
}

// Macro-based tests for consistency
LITERAL_TEST_CASE("True literal", "true", expectBoolLiteral(node, true))
LITERAL_TEST_CASE("False literal", "false", expectBoolLiteral(node, false))
LITERAL_TEST_CASE("Null literal", "null", expectNullLiteral(node))
