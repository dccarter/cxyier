#include "../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Basic identifier expressions", "[parser][identifiers]") {
  SECTION("Simple identifier") {
    auto fixture = createParserFixture("variable");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "variable");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Single letter identifier") {
    auto fixture = createParserFixture("x");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "x");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Identifier with numbers") {
    auto fixture = createParserFixture("var123");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "var123");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Identifier with underscores") {
    auto fixture = createParserFixture("my_variable");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "my_variable");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Identifier starting with underscore") {
    auto fixture = createParserFixture("_private");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "_private");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Long identifier") {
    auto fixture =
        createParserFixture("very_long_variable_name_with_many_words");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "very_long_variable_name_with_many_words");
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Identifier expressions in contexts",
          "[parser][identifiers]") {
  SECTION("Identifier in primary expression") {
    auto fixture = createParserFixture("myVar");
    auto *node = fixture->parsePrimaryExpression();
    expectIdentifier(node, "myVar");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Identifier in expression") {
    auto fixture = createParserFixture("testVariable");
    auto *node = fixture->parseExpression();
    expectIdentifier(node, "testVariable");
    REQUIRE(fixture->isAtEnd());
  }

  SECTION("Identifier in literal expression context fails") {
    auto fixture = createParserFixture("identifier");
    auto *node = fixture->parseLiteralExpression();
    expectParseFailure(node);
  }
}

TEST_CASE("Parser: Multiple identifier expressions", "[parser][identifiers]") {
  auto fixture = createParserFixture("first second third");

  // Parse first identifier
  auto *node1 = fixture->parseIdentifierExpression();
  expectIdentifier(node1, "first");

  // Parse second identifier (parser advanced automatically)
  auto *node2 = fixture->parseIdentifierExpression();
  expectIdentifier(node2, "second");

  // Parse third identifier (parser advanced automatically)
  auto *node3 = fixture->parseIdentifierExpression();
  expectIdentifier(node3, "third");

  // Should be at end
  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Identifier token buffer behavior", "[parser][identifiers]") {
  auto fixture = createParserFixture("myVar 42");

  // Initially at identifier
  REQUIRE(fixture->current().kind == TokenKind::Ident);
  REQUIRE(fixture->lookahead(1).kind == TokenKind::IntLiteral);

  // Parse identifier
  auto *node = fixture->parseIdentifierExpression();
  expectIdentifier(node, "myVar");

  // Should have advanced to next token after parsing
  REQUIRE(fixture->current().kind == TokenKind::IntLiteral);
}

TEST_CASE("Parser: Identifier location information", "[parser][identifiers]") {
  auto fixture = createParserFixture("testId");

  // Store the token location before parsing (parser will advance)
  Location expectedLocation = fixture->current().location;
  auto *node = fixture->parseIdentifierExpression();

  REQUIRE(node != nullptr);
  REQUIRE(node->kind == ast::astIdentifier);

  // Location should be set
  REQUIRE(node->location.isValid());

  // Location should correspond to the original token
  REQUIRE(node->location == expectedLocation);
}

TEST_CASE("Parser: Identifier error cases", "[parser][identifiers]") {
  SECTION("Wrong token type - integer") {
    auto fixture = createParserFixture("42");
    auto *node = fixture->parseIdentifierExpression();
    expectParseFailure(node);
  }

  SECTION("Wrong token type - string") {
    auto fixture = createParserFixture("\"not_an_identifier\"");
    auto *node = fixture->parseIdentifierExpression();
    expectParseFailure(node);
  }

  SECTION("Wrong token type - boolean") {
    auto fixture = createParserFixture("true");
    auto *node = fixture->parseIdentifierExpression();
    expectParseFailure(node);
  }

  SECTION("Empty input") {
    auto fixture = createParserFixture("");
    auto *node = fixture->parseIdentifierExpression();
    expectParseFailure(node);
  }

  SECTION("Keyword instead of identifier") {
    auto fixture = createParserFixture("func");
    auto *node = fixture->parseIdentifierExpression();
    expectParseFailure(node);
  }
}

TEST_CASE("Parser: Identifier vs keyword distinction",
          "[parser][identifiers]") {
  SECTION("Clear identifier") {
    auto fixture = createParserFixture("myFunction");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "myFunction");
  }

  SECTION("Keyword should not parse as identifier") {
    auto fixture = createParserFixture("func");
    auto *node = fixture->parseIdentifierExpression();
    expectParseFailure(node);
  }

  SECTION("Identifier similar to keyword") {
    auto fixture = createParserFixture("function");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "function");
  }

  SECTION("Identifier with keyword prefix") {
    auto fixture = createParserFixture("funcName");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "funcName");
  }
}

TEST_CASE("Parser: Common identifier patterns", "[parser][identifiers]") {
  SECTION("CamelCase identifier") {
    auto fixture = createParserFixture("camelCaseVariable");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "camelCaseVariable");
  }

  SECTION("PascalCase identifier") {
    auto fixture = createParserFixture("PascalCaseType");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "PascalCaseType");
  }

  SECTION("snake_case identifier") {
    auto fixture = createParserFixture("snake_case_variable");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "snake_case_variable");
  }

  SECTION("SCREAMING_SNAKE_CASE identifier") {
    auto fixture = createParserFixture("SCREAMING_SNAKE_CASE");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "SCREAMING_SNAKE_CASE");
  }

  SECTION("Mixed case with numbers") {
    auto fixture = createParserFixture("var1_test2_Value3");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "var1_test2_Value3");
  }
}

TEST_CASE("Parser: Identifier string interning", "[parser][identifiers]") {
  auto fixture = createParserFixture("testName");
  auto *node1 = fixture->parseIdentifierExpression();

  REQUIRE(node1 != nullptr);
  REQUIRE(node1->kind == ast::astIdentifier);

  // Parse the same identifier again in a new fixture
  auto fixture2 = createParserFixture("testName");
  auto *node2 = fixture2->parseIdentifierExpression();

  REQUIRE(node2 != nullptr);
  REQUIRE(node2->kind == ast::astIdentifier);

  // Interned strings should be the same
  auto *identNode1 = static_cast<ast::IdentifierNode *>(node1);
  auto *identNode2 = static_cast<ast::IdentifierNode *>(node2);

  REQUIRE(identNode1->name.view() == identNode2->name.view());
}

TEST_CASE("Parser: Identifiers mixed with literals", "[parser][identifiers]") {
  auto fixture = createParserFixture("var1 42 \"string\" identifier2 true");

  // Test mixed sequence
  auto *node1 = fixture->parsePrimaryExpression();
  expectIdentifier(node1, "var1");

  auto *node2 = fixture->parsePrimaryExpression();
  expectIntegerLiteral(node2, 42);

  auto *node3 = fixture->parsePrimaryExpression();
  expectStringLiteral(node3, "string");

  auto *node4 = fixture->parsePrimaryExpression();
  expectIdentifier(node4, "identifier2");

  auto *node5 = fixture->parsePrimaryExpression();
  expectBoolLiteral(node5, true);

  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Reserved identifier patterns", "[parser][identifiers]") {
  // These should all be valid identifiers (not keywords in CXY)
  std::vector<std::string> identifiers = {
      "public",   "private",  "protected", "namespace", "using",
      "template", "typename", "int",       "float",     "double",
      "long",     "short",    "new",       "malloc",    "free",
      "main",     "printf",   "scanf",     "sizeof"};

  for (const auto &identifier : identifiers) {
    auto fixture = createParserFixture(identifier);
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, identifier);
    REQUIRE(fixture->isAtEnd());
  }
}

TEST_CASE("Parser: Edge case identifier patterns", "[parser][identifiers]") {
  SECTION("Single underscore") {
    auto fixture = createParserFixture("_");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "_");
  }

  SECTION("Multiple underscores") {
    auto fixture = createParserFixture("___");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "___");
  }

  SECTION("Underscore with numbers") {
    auto fixture = createParserFixture("_123");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "_123");
  }

  SECTION("All numbers after valid start") {
    auto fixture = createParserFixture("a123456789");
    auto *node = fixture->parseIdentifierExpression();
    expectIdentifier(node, "a123456789");
  }
}

// Macro-based tests for consistency
IDENTIFIER_TEST_CASE("Simple identifier", "variable", "variable")
IDENTIFIER_TEST_CASE("Single letter", "x", "x")
IDENTIFIER_TEST_CASE("With numbers", "var123", "var123")
IDENTIFIER_TEST_CASE("With underscores", "my_var", "my_var")
IDENTIFIER_TEST_CASE("Starting underscore", "_private", "_private")
IDENTIFIER_TEST_CASE("CamelCase", "camelCase", "camelCase")
IDENTIFIER_TEST_CASE("PascalCase", "PascalCase", "PascalCase")
IDENTIFIER_TEST_CASE("snake_case", "snake_case", "snake_case")
IDENTIFIER_TEST_CASE("UPPER_CASE", "UPPER_CASE", "UPPER_CASE")
