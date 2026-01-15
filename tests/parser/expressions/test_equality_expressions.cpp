#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Equality Expressions", "[parser][expressions][equality]") {

  SECTION("Basic equality operator") {
    ParserTestFixture fixture("a == b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(BinaryExpr == (Identifier a) (Identifier b))");
  }

  SECTION("Basic inequality operator") {
    ParserTestFixture fixture("a != b");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(BinaryExpr != (Identifier a) (Identifier b))");
  }

  SECTION("Left associativity with same precedence") {
    ParserTestFixture fixture("a == b != c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr != (BinaryExpr == (Identifier a) "
                                "(Identifier b)) (Identifier c))");
  }

  SECTION("Multiple equality operators") {
    ParserTestFixture fixture("a == b == c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr == (BinaryExpr == (Identifier a) "
                                "(Identifier b)) (Identifier c))");
  }

  SECTION("Multiple inequality operators") {
    ParserTestFixture fixture("a != b != c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr != (BinaryExpr != (Identifier a) "
                                "(Identifier b)) (Identifier c))");
  }

  SECTION("Mixed with lower precedence - relational") {
    ParserTestFixture fixture("a < b == c > d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr == (BinaryExpr < (Identifier a) "
                                "(Identifier b)) (BinaryExpr > "
                                "(Identifier c) (Identifier d)))");
  }

  SECTION("Mixed with lower precedence - arithmetic") {
    ParserTestFixture fixture("a + b == c * d");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr == (BinaryExpr + (Identifier a) "
                                "(Identifier b)) (BinaryExpr * "
                                "(Identifier c) (Identifier d)))");
  }

  SECTION("Parenthesized equality expressions") {
    ParserTestFixture fixture("(a == b) != c");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr != (BinaryExpr == (Identifier a) "
                                "(Identifier b)) (Identifier c))");
  }

  SECTION("Equality with literals") {
    ParserTestFixture fixture("42 == true");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result, "(BinaryExpr == (Int 42) (Bool true))");
  }

  SECTION("Inequality with string literals") {
    ParserTestFixture fixture("\"hello\" != \"world\"");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(
        result, "(BinaryExpr != (String \"hello\") (String \"world\"))");
  }

  SECTION("Complex nested expression") {
    ParserTestFixture fixture("a + b * c == d << e != f");
    auto result = fixture.parseExpression();

    REQUIRE(result != nullptr);
    REQUIRE_AST_MATCHES(result,
                        "(BinaryExpr != (BinaryExpr == (BinaryExpr + "
                        "(Identifier a) (BinaryExpr * "
                        "(Identifier b) (Identifier c))) (BinaryExpr << "
                        "(Identifier d) (Identifier e))) (Identifier f))");
  }
}
