#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Bitwise Expressions", "[parser][expressions][bitwise]") {

  SECTION("Basic bitwise AND") {
    ParserTestFixture fixture("a & b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr & (Identifier a) (Identifier b))");
  }

  SECTION("Left associativity with multiple AND") {
    ParserTestFixture fixture("a & b & c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr & (BinaryExpr & (Identifier a) "
                                "(Identifier b)) (Identifier c))");
  }

  SECTION("Multiple AND operators") {
    ParserTestFixture fixture("x & y & z & w");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(BinaryExpr & (BinaryExpr & (BinaryExpr & "
                        "(Identifier x) (Identifier y)) (Identifier z)) "
                        "(Identifier w))");
  }

  SECTION("Mixed with lower precedence - equality") {
    ParserTestFixture fixture("a == b & c != d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr & (BinaryExpr == (Identifier a) "
                                "(Identifier b)) (BinaryExpr != "
                                "(Identifier c) (Identifier d)))");
  }

  SECTION("Mixed with lower precedence - relational") {
    ParserTestFixture fixture("a < b & c > d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr & (BinaryExpr < (Identifier a) "
                                "(Identifier b)) (BinaryExpr > "
                                "(Identifier c) (Identifier d)))");
  }

  SECTION("Mixed with lower precedence - arithmetic") {
    ParserTestFixture fixture("a + b & c * d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr & (BinaryExpr + (Identifier a) "
                                "(Identifier b)) (BinaryExpr * "
                                "(Identifier c) (Identifier d)))");
  }

  SECTION("Parenthesized bitwise AND expressions") {
    ParserTestFixture fixture("(a & b) == c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr == (BinaryExpr & (Identifier a) "
                                "(Identifier b)) (Identifier c))");
  }

  SECTION("Bitwise AND with literals") {
    ParserTestFixture fixture("42 & 255");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr & (Int 42) (Int 255))");
  }

  SECTION("Complex nested expression") {
    ParserTestFixture fixture("a + b * c & d << e == f");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr & (BinaryExpr + "
                                "(Identifier a) (BinaryExpr * "
                                "(Identifier b) (Identifier c))) "
                                "(BinaryExpr == (BinaryExpr << (Identifier d) "
                                "(Identifier e)) (Identifier f)))");
  }

  // Bitwise XOR expressions (^)
  SECTION("Basic bitwise XOR") {
    ParserTestFixture fixture("a ^ b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr ^ (Identifier a) (Identifier b))");
  }

  SECTION("Left associativity with multiple XOR") {
    ParserTestFixture fixture("a ^ b ^ c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr ^ (BinaryExpr ^ (Identifier a) "
                                "(Identifier b)) (Identifier c))");
  }

  SECTION("XOR with lower precedence - bitwise AND") {
    ParserTestFixture fixture("a & b ^ c & d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr ^ (BinaryExpr & (Identifier a) "
                                "(Identifier b)) (BinaryExpr & "
                                "(Identifier c) (Identifier d)))");
  }

  SECTION("XOR with literals") {
    ParserTestFixture fixture("15 ^ 7");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr ^ (Int 15) (Int 7))");
  }

  SECTION("Complex XOR expression") {
    ParserTestFixture fixture("a & b ^ c == d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr ^ (BinaryExpr & (Identifier a) "
                                "(Identifier b)) (BinaryExpr == "
                                "(Identifier c) (Identifier d)))");
  }

  // Bitwise OR expressions (|)
  SECTION("Basic bitwise OR") {
    ParserTestFixture fixture("a | b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr | (Identifier a) (Identifier b))");
  }

  SECTION("Left associativity with multiple OR") {
    ParserTestFixture fixture("a | b | c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr | (BinaryExpr | (Identifier a) "
                                "(Identifier b)) (Identifier c))");
  }

  SECTION("OR with lower precedence - bitwise XOR") {
    ParserTestFixture fixture("a ^ b | c ^ d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr | (BinaryExpr ^ (Identifier a) "
                                "(Identifier b)) (BinaryExpr ^ "
                                "(Identifier c) (Identifier d)))");
  }

  SECTION("OR with literals") {
    ParserTestFixture fixture("8 | 4");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr | (Int 8) (Int 4))");
  }

  SECTION("Complex OR expression") {
    ParserTestFixture fixture("a & b ^ c | d == e");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr | (BinaryExpr ^ (BinaryExpr & "
                                "(Identifier a) (Identifier b)) "
                                "(Identifier c)) (BinaryExpr == "
                                "(Identifier d) (Identifier e)))");
  }
}
