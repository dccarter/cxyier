#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Basic range expressions", "[parser][expressions][range]") {
  SECTION("Inclusive range") {
    auto fixture = createParserFixture("1..10");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(RangeExpr .. (Int 1) (Int 10))");
  }

  SECTION("Exclusive range") {
    auto fixture = createParserFixture("1..<10");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(RangeExpr ..< (Int 1) (Int 10))");
  }

  SECTION("Range with identifiers") {
    auto fixture = createParserFixture("start..end");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(RangeExpr .. (Identifier start) (Identifier end))");
  }

  SECTION("Range with mixed types") {
    auto fixture = createParserFixture("0..count");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(RangeExpr .. (Int 0) (Identifier count))");
  }
}

TEST_CASE("Parser: Open range expressions", "[parser][expressions][range]") {
  SECTION("Open start range") {
    auto fixture = createParserFixture("..10");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(RangeExpr .. (Int 10))");
  }

  SECTION("Open end range") {
    auto fixture = createParserFixture("5..");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(RangeExpr .. (Int 5))");
  }

  SECTION("Full open range") {
    auto fixture = createParserFixture("..");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(RangeExpr ..)");
  }

  SECTION("Open start exclusive range") {
    auto fixture = createParserFixture("..<10");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(RangeExpr ..< (Int 10))");
  }
}

TEST_CASE("Parser: Function-style range expressions",
          "[parser][expressions][range][function]") {
  SECTION("Single argument range") {
    auto fixture = createParserFixture("makeRange(10)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(CallExpr (Identifier makeRange) (Int 10))");
  }

  SECTION("Two argument range") {
    auto fixture = createParserFixture("makeRange(1, 10)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(CallExpr (Identifier makeRange) (Int 1) (Int 10))");
  }

  SECTION("Three argument range with step") {
    auto fixture = createParserFixture("makeRange(0, 100, 2)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(CallExpr (Identifier makeRange) (Int 0) (Int 100) (Int 2))");
  }

  SECTION("Range with variables") {
    auto fixture = createParserFixture("makeRange(start, end, step)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(CallExpr (Identifier makeRange) (Identifier start) "
                        "(Identifier end) (Identifier step))");
  }
}

TEST_CASE("Parser: Range expressions with complex expressions",
          "[parser][expressions][range]") {
  SECTION("Range with arithmetic expressions") {
    auto fixture = createParserFixture("x + 1..y - 1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(RangeExpr .. (BinaryExpr + (Identifier x) (Int "
                              "1)) (BinaryExpr - (Identifier y) (Int 1)))");
  }

  SECTION("Range with member access") {
    auto fixture = createParserFixture("obj.start..obj.end");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr,
        "(RangeExpr .. (MemberExpr . (Identifier obj) (Identifier start)) "
        "(MemberExpr . (Identifier obj) (Identifier end)))");
  }

  SECTION("Range with function calls") {
    auto fixture = createParserFixture("getStart()..getEnd()");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(RangeExpr .. (CallExpr (Identifier getStart)) "
                              "(CallExpr (Identifier getEnd)))");
  }

  SECTION("Range with array indexing") {
    auto fixture = createParserFixture("arr[0]..arr[len-1]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(RangeExpr .. (IndexExpr (Identifier arr) (Int 0)) (IndexExpr "
              "(Identifier arr) (BinaryExpr - (Identifier len) (Int 1))))");
  }
}

TEST_CASE("Parser: Range expressions in context",
          "[parser][expressions][range][context]") {
  SECTION("Range as array index") {
    auto fixture = createParserFixture("array[1..5]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(IndexExpr (Identifier array) (RangeExpr .. (Int 1) (Int 5)))");
  }

  SECTION("Range as function argument") {
    auto fixture = createParserFixture("process(0..10)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr,
        "(CallExpr (Identifier process) (RangeExpr .. (Int 0) (Int 10)))");
  }

  SECTION("Range in assignment") {
    auto fixture = createParserFixture("slice = data[start..end]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(AssignmentExpr = (Identifier slice) (IndexExpr (Identifier "
              "data) (RangeExpr .. (Identifier start) (Identifier end))))");
  }

  SECTION("Multiple ranges as arguments") {
    auto fixture = createParserFixture("copy(src[0..5], dst[10..15])");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(CallExpr (Identifier copy) (IndexExpr (Identifier "
                        "src) (RangeExpr .. (Int 0) (Int 5))) (IndexExpr "
                        "(Identifier dst) (RangeExpr .. (Int 10) (Int 15))))");
  }
}

TEST_CASE("Parser: Range expressions with precedence",
          "[parser][expressions][range][precedence]") {
  SECTION("Range with addition - left side") {
    auto fixture = createParserFixture("x + 1..10");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(RangeExpr .. (BinaryExpr + (Identifier x) (Int 1)) (Int 10))");
  }

  SECTION("Range with addition - right side") {
    auto fixture = createParserFixture("1..x + 10");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(RangeExpr .. (Int 1) (BinaryExpr + (Identifier x) (Int 10)))");
  }

  SECTION("Range with multiplication") {
    auto fixture = createParserFixture("start * 2..end * 2");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(RangeExpr .. (BinaryExpr * (Identifier start) (Int "
                        "2)) (BinaryExpr * (Identifier end) (Int 2)))");
  }

  SECTION("Range with comparison") {
    auto fixture = createParserFixture("1..10 == other");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr,
        "(BinaryExpr == (RangeExpr .. (Int 1) (Int 10)) (Identifier other))");
  }
}

TEST_CASE("Parser: Range expression error cases",
          "[parser][expressions][range][errors]") {
  SECTION("Incomplete range - missing end") {
    auto fixture = createParserFixture("1..<");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Invalid range operator") {
    auto fixture = createParserFixture("1..=10");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Empty range function call") {
    auto fixture = createParserFixture("range()");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Too many range function arguments") {
    auto fixture = createParserFixture("range(1, 2, 3, 4)");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }
}

TEST_CASE("Parser: Range expressions with whitespace variations",
          "[parser][expressions][range]") {
  SECTION("Extra whitespace around range operator") {
    auto fixture = createParserFixture("1  ..  10");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(RangeExpr .. (Int 1) (Int 10))");
  }

  SECTION("Extra whitespace around exclusive range") {
    auto fixture = createParserFixture("1  ..<  10");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(RangeExpr ..< (Int 1) (Int 10))");
  }

  SECTION("No whitespace around range operator") {
    auto fixture = createParserFixture("start..end");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(RangeExpr .. (Identifier start) (Identifier end))");
  }

  SECTION("Mixed whitespace in open ranges") {
    auto fixture = createParserFixture("  ..  10");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(RangeExpr .. (Int 10))");
  }
}

TEST_CASE("Parser: Nested and chained range expressions",
          "[parser][expressions][range][complex]") {
  SECTION("Range in parentheses") {
    auto fixture = createParserFixture("(1..10)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(RangeExpr .. (Int 1) (Int 10))");
  }

  SECTION("Range with casts") {
    auto fixture = createParserFixture("start as i32..end as i32");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(RangeExpr .. (CastExpr as (Identifier start) (Type "
                        "i32)) (CastExpr as (Identifier end) (Type i32)))");
  }

  SECTION("Range in struct literal") {
    auto fixture = createParserFixture("{ values: 0..10 }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StructExpr (FieldExpr (Identifier values) "
                              "(RangeExpr .. (Int 0) (Int 10))))");
  }

  SECTION("Range in array literal") {
    auto fixture = createParserFixture("[1..5, 10..15]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(ArrayExpr (RangeExpr .. (Int 1) (Int 5)) "
                              "(RangeExpr .. (Int 10) (Int 15)))");
  }
}
