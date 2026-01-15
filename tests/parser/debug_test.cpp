#include "../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Debug: Token advancement tracing", "[debug]") {
  auto fixture = createParserFixture("10 20 30");

  // Check initial state
  REQUIRE(fixture->current().kind == TokenKind::IntLiteral);
  REQUIRE(fixture->current().getIntValue() == 10);
  REQUIRE(fixture->lookahead(1).kind == TokenKind::IntLiteral);
  REQUIRE(fixture->lookahead(1).getIntValue() == 20);
  REQUIRE(fixture->lookahead(2).kind == TokenKind::IntLiteral);
  REQUIRE(fixture->lookahead(2).getIntValue() == 30);

  // Parse first integer
  auto *node1 = fixture->parseLiteralExpression();
  REQUIRE(node1 != nullptr);
  REQUIRE(node1->kind == ast::astInt);
  auto *intNode1 = static_cast<ast::IntLiteralNode *>(node1);
  REQUIRE(intNode1->value == 10);

  // Check state after first parse
  REQUIRE(fixture->current().kind == TokenKind::IntLiteral);
  REQUIRE(fixture->current().getIntValue() == 20);

  // Parse second integer
  auto *node2 = fixture->parseLiteralExpression();
  REQUIRE(node2 != nullptr);
  REQUIRE(node2->kind == ast::astInt);
  auto *intNode2 = static_cast<ast::IntLiteralNode *>(node2);
  REQUIRE(intNode2->value == 20);

  // Check state after second parse
  REQUIRE(fixture->current().kind == TokenKind::IntLiteral);
  REQUIRE(fixture->current().getIntValue() == 30);

  // Parse third integer
  auto *node3 = fixture->parseLiteralExpression();
  REQUIRE(node3 != nullptr);
  REQUIRE(node3->kind == ast::astInt);
  auto *intNode3 = static_cast<ast::IntLiteralNode *>(node3);
  REQUIRE(intNode3->value == 30);

  // Should be at end
  REQUIRE(fixture->isAtEnd());
}
