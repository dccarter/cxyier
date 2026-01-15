#include "../../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Simple member access", "[parser][expressions][member]") {
  SECTION("Basic field access") {
    auto fixture = createParserFixture("obj.field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(MemberExpr . (Identifier obj) (Identifier field))");
  }

  SECTION("Numeric field access") {
    auto fixture = createParserFixture("obj.0");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MemberExpr . (Identifier obj) (Int 0))");
  }

  SECTION("Multi-digit numeric access") {
    auto fixture = createParserFixture("tuple.42");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, "(MemberExpr . (Identifier tuple) (Int 42))");
  }

  SECTION("Complex expression as object") {
    auto fixture = createParserFixture("(x + y).field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr .
        (BinaryExpr +
          (Identifier x)
          (Identifier y))
        (Identifier field))
    )");
  }
}

TEST_CASE("Parser: Overloaded member access (&.)",
          "[parser][expressions][member]") {
  SECTION("Basic overloaded access") {
    auto fixture = createParserFixture("obj&.field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr &.
        (Identifier obj)
        (Identifier field))
    )");
  }

  SECTION("Chained overloaded access") {
    auto fixture = createParserFixture("obj&.inner&.field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr &.
        (MemberExpr &.
          (Identifier obj)
          (Identifier inner))
        (Identifier field))
    )");
  }
}

TEST_CASE("Parser: Chained member access", "[parser][expressions][member]") {
  SECTION("Simple chaining") {
    auto fixture = createParserFixture("obj.inner.field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr .
        (MemberExpr .
          (Identifier obj)
          (Identifier inner))
        (Identifier field))
    )");
  }

  SECTION("Mixed member and numeric access") {
    auto fixture = createParserFixture("obj.tuple.0");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr .
        (MemberExpr .
          (Identifier obj)
          (Identifier tuple))
        (Int 0))
    )");
  }

  SECTION("Deep chaining") {
    auto fixture = createParserFixture("a.b.c.d.e");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr .
        (MemberExpr .
          (MemberExpr .
            (MemberExpr .
              (Identifier a)
              (Identifier b))
            (Identifier c))
          (Identifier d))
        (Identifier e))
    )");
  }
}

TEST_CASE("Parser: Member access with complex expressions",
          "[parser][expressions][member]") {
  SECTION("Member access on function result") {
    auto fixture = createParserFixture("getObject().field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr .
        (CallExpr (Identifier getObject))
        (Identifier field))
    )");
  }

  SECTION("Member access on array literal") {
    auto fixture = createParserFixture("[obj1, obj2].0");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr .
        (ArrayExpr (Identifier obj1) (Identifier obj2))
        (Int 0))
    )");
  }

  SECTION("Member access on tuple literal") {
    auto fixture = createParserFixture("(x, y, z).1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr .
        (TupleExpr (Identifier x) (Identifier y) (Identifier z))
        (Int 1))
    )");
  }

  SECTION("Member access on indexed result") {
    auto fixture = createParserFixture("array[0].field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr .
        (IndexExpr
          (Identifier array)
          (Int 0))
        (Identifier field))
    )");
  }
}

TEST_CASE("Parser: Member access with whitespace variations",
          "[parser][expressions][member]") {
  SECTION("No spaces around dot") {
    auto fixture = createParserFixture("obj.field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(MemberExpr . (Identifier obj) (Identifier field))");
  }

  SECTION("Spaces around dot (whitespace allowed)") {
    // Language allows whitespace around operators
    auto fixture = createParserFixture("obj . field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr,
                        "(MemberExpr . (Identifier obj) (Identifier field))");
  }

  SECTION("Newlines in chained access") {
    auto fixture = createParserFixture("obj\n  .field\n  .method");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr .
        (MemberExpr .
          (Identifier obj)
          (Identifier field))
        (Identifier method))
    )");
  }
}

TEST_CASE("Parser: Member access precedence",
          "[parser][expressions][member][precedence]") {
  SECTION("Member access has higher precedence than arithmetic") {
    auto fixture = createParserFixture("obj.field + 1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (MemberExpr .
          (Identifier obj)
          (Identifier field))
        (Int 1))
    )");
  }

  SECTION("Arithmetic expression as object") {
    auto fixture = createParserFixture("(a + b).field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr .
        (BinaryExpr +
          (Identifier a)
          (Identifier b))
        (Identifier field))
    )");
  }

  SECTION("Complex precedence with member access") {
    auto fixture = createParserFixture("obj.field * array.length + 1");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (BinaryExpr +
        (BinaryExpr *
          (MemberExpr .
            (Identifier obj)
            (Identifier field))
          (MemberExpr .
            (Identifier array)
            (Identifier length)))
        (Int 1))
    )");
  }
}

TEST_CASE("Parser: Mixed member access, indexing, and function calls",
          "[parser][expressions][member][integration]") {
  SECTION("Method call") {
    auto fixture = createParserFixture("obj.method()");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (MemberExpr .
          (Identifier obj)
          (Identifier method)))
    )");
  }

  SECTION("Method call with arguments") {
    auto fixture = createParserFixture("obj.method(arg1, arg2)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (MemberExpr .
          (Identifier obj)
          (Identifier method))
        (Identifier arg1)
        (Identifier arg2))
    )");
  }

  SECTION("Chained method calls") {
    auto fixture = createParserFixture("obj.getInner().method()");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (CallExpr
        (MemberExpr .
          (CallExpr
            (MemberExpr .
              (Identifier obj)
              (Identifier getInner)))
          (Identifier method)))
    )");
  }

  SECTION("Member access on indexed array") {
    auto fixture = createParserFixture("objects[i].field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr .
        (IndexExpr
          (Identifier objects)
          (Identifier i))
        (Identifier field))
    )");
  }

  SECTION("Complex chaining") {
    auto fixture = createParserFixture("getObjects()[0].method().result.field");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (MemberExpr .
        (MemberExpr .
          (CallExpr
            (MemberExpr .
              (IndexExpr
                (CallExpr (Identifier getObjects))
                (Int 0))
              (Identifier method)))
          (Identifier result))
        (Identifier field))
    )");
  }

  SECTION("Indexing member access result") {
    auto fixture = createParserFixture("obj.array[i]");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_STRUCTURALLY_MATCHES(expr, R"(
      (IndexExpr
        (MemberExpr .
          (Identifier obj)
          (Identifier array))
        (Identifier i))
    )");
  }

  SECTION("Function call with member access arguments") {
    auto fixture = createParserFixture("myFunc(obj.field, array.0)");
    auto *expr = fixture->parseExpression();
    REQUIRE_AST_MATCHES(expr, R"(
      (CallExpr
        (Identifier myFunc)
        (MemberExpr .
          (Identifier obj)
          (Identifier field))
        (MemberExpr .
          (Identifier array)
          (Int 0)))
    )");
  }
}

TEST_CASE("Parser: Member access error cases",
          "[parser][expressions][member][error]") {
  SECTION("Missing member name") {
    auto fixture = createParserFixture("obj.");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }

  SECTION("Invalid member expression") {
    auto fixture = createParserFixture("obj.,");
    auto *expr = fixture->parseExpression();
    REQUIRE(expr == nullptr);
    REQUIRE(fixture->hasErrors());
  }
}
