#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Empty struct literals",
          "[parser][expressions][struct][literals]") {
  SECTION("Empty typed struct") {
    auto fixture = createParserFixture("Point {}");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StructExpr (Identifier Point))");
  }

  SECTION("Empty typed struct with spaces") {
    auto fixture = createParserFixture("Point { }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StructExpr (Identifier Point))");
  }
}

TEST_CASE("Parser: Anonymous struct literals",
          "[parser][expressions][struct][literals]") {
  SECTION("Single field with value") {
    auto fixture = createParserFixture("{ x: 42 }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(StructExpr (FieldExpr (Identifier x) (Int 42)))");
  }

  SECTION("Multiple fields with values") {
    auto fixture = createParserFixture("{ x: 1, y: 2 }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StructExpr (FieldExpr (Identifier x) (Int 1)) "
                              "(FieldExpr (Identifier y) (Int 2)))");
  }

  SECTION("Single shorthand field") {
    auto fixture = createParserFixture("{ x }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(StructExpr (FieldExpr (Identifier x) (Identifier x)))");
  }

  SECTION("Multiple shorthand fields") {
    auto fixture = createParserFixture("{ x, y }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(StructExpr (FieldExpr (Identifier x) (Identifier x)) "
                        "(FieldExpr (Identifier y) (Identifier y)))");
  }

  SECTION("Mixed explicit and shorthand fields") {
    auto fixture = createParserFixture("{ x: 42, y }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StructExpr (FieldExpr (Identifier x) (Int 42)) "
                              "(FieldExpr (Identifier y) (Identifier y)))");
  }
}

TEST_CASE("Parser: Typed struct literals",
          "[parser][expressions][struct][literals]") {
  SECTION("Single field with value") {
    auto fixture = createParserFixture("Point { x: 42 }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr,
        "(StructExpr (Identifier Point) (FieldExpr (Identifier x) (Int 42)))");
  }

  SECTION("Multiple fields with values") {
    auto fixture = createParserFixture("Point { x: 1, y: 2 }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(StructExpr (Identifier Point) (FieldExpr (Identifier "
                        "x) (Int 1)) (FieldExpr (Identifier y) (Int 2)))");
  }

  SECTION("Single shorthand field") {
    auto fixture = createParserFixture("Point { x }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StructExpr (Identifier Point) (FieldExpr "
                              "(Identifier x) (Identifier x)))");
  }

  SECTION("Multiple shorthand fields") {
    auto fixture = createParserFixture("Point { x, y }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(StructExpr (Identifier Point) (FieldExpr (Identifier x) "
              "(Identifier x)) (FieldExpr (Identifier y) (Identifier y)))");
  }

  SECTION("Mixed explicit and shorthand fields") {
    auto fixture = createParserFixture("Point { x: 42, y }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(StructExpr (Identifier Point) (FieldExpr (Identifier x) (Int "
              "42)) (FieldExpr (Identifier y) (Identifier y)))");
  }
}

TEST_CASE("Parser: Struct literals with complex expressions",
          "[parser][expressions][struct][literals]") {
  SECTION("Field with string value") {
    auto fixture = createParserFixture("{ name: \"John\" }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(StructExpr (FieldExpr (Identifier name) (String \"John\")))");
  }

  SECTION("Field with float value") {
    auto fixture = createParserFixture("{ pi: 3.14 }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(StructExpr (FieldExpr (Identifier pi) (Float 3.14)))");
  }

  SECTION("Field with binary expression") {
    auto fixture = createParserFixture("{ sum: 2 + 3 }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StructExpr (FieldExpr (Identifier sum) "
                              "(BinaryExpr + (Int 2) (Int 3))))");
  }

  SECTION("Field with member access") {
    auto fixture = createParserFixture("{ value: obj.field }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(StructExpr (FieldExpr (Identifier value) (MemberExpr . "
              "(Identifier obj) (Identifier field))))");
  }

  SECTION("Field with function call") {
    auto fixture = createParserFixture("{ result: myFunc() }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StructExpr (FieldExpr (Identifier result) "
                              "(CallExpr (Identifier myFunc))))");
  }
}

TEST_CASE("Parser: Nested struct literals",
          "[parser][expressions][struct][literals]") {
  SECTION("Anonymous struct with nested anonymous struct") {
    auto fixture = createParserFixture("{ outer: { inner: 42 } }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(StructExpr (FieldExpr (Identifier outer) (StructExpr "
                        "(FieldExpr (Identifier inner) (Int 42)))))");
  }

  SECTION("Typed struct with nested typed struct") {
    auto fixture =
        createParserFixture("Person { address: Address { street: \"Main\" } }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(StructExpr (Identifier Person) (FieldExpr "
                        "(Identifier address) (StructExpr (Identifier Address) "
                        "(FieldExpr (Identifier street) (String \"Main\")))))");
  }

  SECTION("Mixed typed and anonymous nesting") {
    auto fixture = createParserFixture("Person { location: { x: 1, y: 2 } }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(StructExpr (Identifier Person) (FieldExpr (Identifier "
              "location) (StructExpr (FieldExpr (Identifier x) (Int 1)) "
              "(FieldExpr (Identifier y) (Int 2)))))");
  }
}

TEST_CASE("Parser: Struct literals with trailing commas",
          "[parser][expressions][struct][literals]") {
  SECTION("Single field with trailing comma") {
    auto fixture = createParserFixture("{ x: 42, }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(StructExpr (FieldExpr (Identifier x) (Int 42)))");
  }

  SECTION("Multiple fields with trailing comma") {
    auto fixture = createParserFixture("{ x: 1, y: 2, }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StructExpr (FieldExpr (Identifier x) (Int 1)) "
                              "(FieldExpr (Identifier y) (Int 2)))");
  }

  SECTION("Typed struct with trailing comma") {
    auto fixture = createParserFixture("Point { x: 1, y: 2, }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(StructExpr (Identifier Point) (FieldExpr (Identifier "
                        "x) (Int 1)) (FieldExpr (Identifier y) (Int 2)))");
  }
}

TEST_CASE("Parser: Struct literal error cases",
          "[parser][expressions][struct][literals][errors]") {
  SECTION("Empty anonymous struct is not allowed") {
    auto fixture = createParserFixture("{}");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Missing colon in named field") {
    auto fixture = createParserFixture("{ x 42 }");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Missing field value after colon") {
    auto fixture = createParserFixture("{ x: }");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Missing closing brace") {
    auto fixture = createParserFixture("{ x: 42");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Missing comma between fields") {
    auto fixture = createParserFixture("{ x: 1 y: 2 }");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }

  SECTION("Invalid field name") {
    auto fixture = createParserFixture("{ 123: 42 }");
    auto *expr = fixture->parseExpression();
    expectParseFailure(expr);
  }
}

TEST_CASE("Parser: Struct literals with whitespace variations",
          "[parser][expressions][struct][literals]") {
  SECTION("Extra whitespace around braces") {
    auto fixture = createParserFixture(" {  x: 42  } ");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(StructExpr (FieldExpr (Identifier x) (Int 42)))");
  }

  SECTION("Extra whitespace around colons") {
    auto fixture = createParserFixture("{ x  :  42 }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(StructExpr (FieldExpr (Identifier x) (Int 42)))");
  }

  SECTION("Extra whitespace around commas") {
    auto fixture = createParserFixture("{ x: 1  ,  y: 2 }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StructExpr (FieldExpr (Identifier x) (Int 1)) "
                              "(FieldExpr (Identifier y) (Int 2)))");
  }

  SECTION("Typed struct with extra whitespace") {
    auto fixture = createParserFixture("Point  {  x: 1  ,  y: 2  }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(StructExpr (Identifier Point) (FieldExpr (Identifier "
                        "x) (Int 1)) (FieldExpr (Identifier y) (Int 2)))");
  }
}
