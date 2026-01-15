#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Basic macro calls", "[parser][expressions][macro]") {
  SECTION("Bare macro call") {
    auto fixture = createParserFixture("println!");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MacroCallExpr (Identifier println))");
  }

  SECTION("Macro call with empty parentheses") {
    auto fixture = createParserFixture("debug!()");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MacroCallExpr (Identifier debug))");
  }

  SECTION("Macro call with single argument") {
    auto fixture = createParserFixture("println!(\"Hello\")");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(MacroCallExpr (Identifier println) (String \"Hello\"))");
  }

  SECTION("Macro call with multiple arguments") {
    auto fixture = createParserFixture("format!(\"Hello {}\", name)");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Macro call with complex identifier") {
    auto fixture = createParserFixture("my_debug_macro!()");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MacroCallExpr (Identifier my_debug_macro))");
  }
}

TEST_CASE("Parser: Macro calls with various argument types",
          "[parser][expressions][macro]") {
  SECTION("Macro with integer argument") {
    auto fixture = createParserFixture("assert!(42)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MacroCallExpr (Identifier assert) (Int 42))");
  }

  SECTION("Macro with boolean argument") {
    auto fixture = createParserFixture("assert!(true)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(MacroCallExpr (Identifier assert) (Bool true))");
  }

  SECTION("Macro with identifier argument") {
    auto fixture = createParserFixture("debug!(variable)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(MacroCallExpr (Identifier debug) (Identifier variable))");
  }

  SECTION("Macro with function call argument") {
    auto fixture = createParserFixture("assert!(getValue())");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr,
        "(MacroCallExpr (Identifier assert) (CallExpr (Identifier getValue)))");
  }

  SECTION("Macro with binary expression argument") {
    auto fixture = createParserFixture("assert!(x > 0)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MacroCallExpr (Identifier assert) (BinaryExpr "
                              "> (Identifier x) (Int 0)))");
  }

  SECTION("Macro with member access argument") {
    auto fixture = createParserFixture("debug!(obj.field)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MacroCallExpr (Identifier debug) (MemberExpr . "
                              "(Identifier obj) (Identifier field)))");
  }
}

TEST_CASE("Parser: Macro calls with interpolated strings",
          "[parser][expressions][macro]") {
  SECTION("Macro with simple interpolated string") {
    auto fixture = createParserFixture("println!(\"Hello {name}!\")");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MacroCallExpr (Identifier println) (StringExpr "
                              "\"Hello \" (Identifier name) \"!\"))");
  }

  SECTION("Macro with complex interpolated string") {
    auto fixture = createParserFixture("debug!(\"Value: {getValue()}\")");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MacroCallExpr (Identifier debug) (StringExpr "
                              "\"Value: \" (CallExpr (Identifier getValue))))");
  }

  SECTION("Macro with nested interpolated strings") {
    auto fixture = createParserFixture(
        "format!(\"Hello {getGreeting(\"User {name}\")}\")");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr,
        "(MacroCallExpr (Identifier format) (StringExpr \"Hello \" (CallExpr "
        "(Identifier getGreeting) (StringExpr \"User \" (Identifier name)))))");
  }
}

TEST_CASE("Parser: Macro calls with collection arguments",
          "[parser][expressions][macro]") {
  SECTION("Macro with array argument") {
    auto fixture = createParserFixture("debug!([1, 2, 3])");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MacroCallExpr (Identifier debug) (ArrayExpr "
                              "(Int 1) (Int 2) (Int 3)))");
  }

  SECTION("Macro with tuple argument") {
    auto fixture = createParserFixture("debug!((a, b, c))");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MacroCallExpr (Identifier debug) (TupleExpr "
                              "(Identifier a) (Identifier b) (Identifier c)))");
  }

  SECTION("Macro with struct argument") {
    auto fixture = createParserFixture("debug!(Point { x: 1, y: 2 })");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MacroCallExpr (Identifier debug) (StructExpr "
                              "(Identifier Point) (FieldExpr (Identifier x) "
                              "(Int 1)) (FieldExpr (Identifier y) (Int 2))))");
  }
}

TEST_CASE("Parser: Macro call precedence and chaining",
          "[parser][expressions][macro]") {
  SECTION("Macro call with member access") {
    auto fixture = createParserFixture("getMacro!().field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MemberExpr . (MacroCallExpr (Identifier "
                              "getMacro)) (Identifier field))");
  }

  SECTION("Macro call with indexing") {
    auto fixture = createParserFixture("getArray!()[0]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(IndexExpr (MacroCallExpr (Identifier getArray)) (Int 0))");
  }

  SECTION("Macro call with function call") {
    auto fixture = createParserFixture("getValue!()()");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(CallExpr (MacroCallExpr (Identifier getValue)))");
  }

  SECTION("Chained macro and member access") {
    auto fixture = createParserFixture("getObject!().method().field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr,
        "(MemberExpr . (CallExpr (MemberExpr . (MacroCallExpr (Identifier "
        "getObject)) (Identifier method))) (Identifier field))");
  }

  SECTION("Macro in binary expression") {
    auto fixture = createParserFixture("getValue!() + 5");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(BinaryExpr + (MacroCallExpr (Identifier getValue)) (Int 5))");
  }
}

TEST_CASE("Parser: Macro calls in various contexts",
          "[parser][expressions][macro]") {
  SECTION("Macro as function argument") {
    auto fixture = createParserFixture("process(getValue!())");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(CallExpr (Identifier process) (MacroCallExpr "
                              "(Identifier getValue)))");
  }

  SECTION("Macro in array literal") {
    auto fixture = createParserFixture("[getValue!(), getOther!()]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(ArrayExpr (MacroCallExpr (Identifier getValue)) "
                        "(MacroCallExpr (Identifier getOther)))");
  }

  SECTION("Macro in tuple literal") {
    auto fixture = createParserFixture("(getValue!(), 42)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(TupleExpr (MacroCallExpr (Identifier getValue)) (Int 42))");
  }

  SECTION("Macro as struct field value") {
    auto fixture = createParserFixture("{ value: getValue!() }");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(StructExpr (FieldExpr (Identifier value) "
                              "(MacroCallExpr (Identifier getValue))))");
  }

  SECTION("Macro in assignment") {
    auto fixture = createParserFixture("x = getValue!()");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(AssignmentExpr = (Identifier x) (MacroCallExpr "
                              "(Identifier getValue)))");
  }
}

TEST_CASE("Parser: Macro call edge cases", "[parser][expressions][macro]") {
  SECTION("Nested macro calls") {
    auto fixture = createParserFixture("outer!(inner!())");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MacroCallExpr (Identifier outer) "
                              "(MacroCallExpr (Identifier inner)))");
  }

  SECTION("Macro with trailing comma") {
    auto fixture = createParserFixture("debug!(value,)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(MacroCallExpr (Identifier debug) (Identifier value))");
  }

  SECTION("Macro with multiple trailing commas") {
    auto fixture = createParserFixture("debug!(a, b,)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr,
        "(MacroCallExpr (Identifier debug) (Identifier a) (Identifier b))");
  }

  SECTION("Macro with whitespace") {
    auto fixture = createParserFixture("debug!( value )");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(
        expr, "(MacroCallExpr (Identifier debug) (Identifier value))");
  }

  SECTION("Bare macro equivalent to empty parentheses") {
    auto fixture1 = createParserFixture("debug!");
    auto *expr1 = fixture1->parseExpression();

    auto fixture2 = createParserFixture("debug!()");
    auto *expr2 = fixture2->parseExpression();

    // Both should have the same structure (no arguments)
    REQUIRE_AST_MATCHES(expr1, "(MacroCallExpr (Identifier debug))");
    REQUIRE_AST_MATCHES(expr2, "(MacroCallExpr (Identifier debug))");
  }
}

TEST_CASE("Parser: Macro call error cases", "[parser][expressions][macro]") {
  SECTION("Invalid macro name") {
    auto fixture = createParserFixture("123!");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(Int 123)");
  }

  SECTION("Unclosed macro arguments") {
    auto fixture = createParserFixture("debug!(value");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Invalid expression in macro arguments") {
    auto fixture = createParserFixture("debug!(+)");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Macro call with extra parentheses") {
    auto fixture = createParserFixture("debug!()()");
    auto *expr = fixture->parseExpression();
    // This should parse as macro call followed by function call
    REQUIRE_AST_MATCHES(expr, "(CallExpr (MacroCallExpr (Identifier debug)))");
  }

  SECTION("Missing identifier before exclamation") {
    auto fixture = createParserFixture("!(a)");
    auto *expr = fixture->parseExpression();
    // This should parse as unary not operator
    REQUIRE_AST_MATCHES(expr, "(UnaryExpr ! (Identifier a))");
  }
}
