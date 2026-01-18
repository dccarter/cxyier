#include "catch2.hpp"
#include "../../parser_test_utils.hpp"
#include "cxy/ast/declarations.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/expressions.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/types.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::test;

TEST_CASE("Qualified Path Parsing - Simple Paths", "[parser][types][qualified-paths][simple]") {
    SECTION("Simple identifier") {
        auto fixture = createParserFixture("Type");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astIdentifier);

        auto *ident = static_cast<IdentifierNode *>(expr);
        REQUIRE(ident->name.view() == "Type");
    }

    SECTION("Module-scoped type") {
        auto fixture = createParserFixture("mod.Type");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astQualifiedPath);

        REQUIRE_AST_MATCHES(expr, R"((QualifiedPath
  (PathSegment mod)
  (PathSegment Type)))");
    }

    SECTION("Deeply nested path") {
        auto fixture = createParserFixture("parent.child.Type");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astQualifiedPath);

        REQUIRE_AST_MATCHES(expr, R"((QualifiedPath
  (PathSegment parent)
  (PathSegment child)
  (PathSegment Type)))");
    }
}

TEST_CASE("Qualified Path Parsing - Generic Type Arguments", "[parser][types][qualified-paths][generics]") {
    SECTION("Simple generic type") {
        auto fixture = createParserFixture("Vector<i32>");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        // Should be a generic instantiation node
        REQUIRE_AST_MATCHES(expr, R"((QualifiedPath
  (PathSegment Vector
    (Type i32))))");
    }

    SECTION("Multiple type arguments") {
        auto fixture = createParserFixture("Map<string, i32>");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE_AST_MATCHES(expr, R"((QualifiedPath
  (PathSegment Map
    (Type string)
    (Type i32))))");
    }

    SECTION("Module-scoped generic") {
        auto fixture = createParserFixture("collections.Vector<i32>");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE_AST_MATCHES(expr, R"((QualifiedPath
  (PathSegment collections)
  (PathSegment Vector
    (Type i32))))");
    }

    SECTION("Nested generic types") {
        auto fixture = createParserFixture("Result<Option<i32>, Error>");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE_AST_MATCHES(expr, R"((QualifiedPath
  (PathSegment Result
    (QualifiedPath
      (PathSegment Option
        (Type i32)))
    (Identifier Error))))");
    }

    SECTION("Complex nested path with generics") {
        auto fixture = createParserFixture("mod.Type<Other<V>>");
        auto *expr = fixture->parseTypeExpression();

        if (!expr) {
            auto diagnostics = fixture->getDiagnostics();
            for (const auto& diag : diagnostics) {
                std::cout << "Error: " << diag.message << " at line " << diag.primaryLocation.start.row << std::endl;
            }
        }

        REQUIRE(expr != nullptr);
        REQUIRE_AST_MATCHES(expr, R"((QualifiedPath
  (PathSegment mod)
  (PathSegment Type
    (QualifiedPath
      (PathSegment Other
        (Identifier V))))))");
    }
}

TEST_CASE("Qualified Path Parsing - Expression Context with ::", "[parser][types][qualified-paths][expressions]") {
    SECTION(":: prefixed type in expression") {
        auto fixture = createParserFixture("::Vector<i32>");
        auto *expr = fixture->parseExpression();

        REQUIRE(expr != nullptr);
        REQUIRE_AST_MATCHES(expr, R"((QualifiedPath
  (PathSegment Vector
    (Type i32))))");
    }

    SECTION(":: prefixed module path") {
        auto fixture = createParserFixture("::collections.HashMap<string, i32>");
        auto *expr = fixture->parseExpression();

        REQUIRE(expr != nullptr);
        REQUIRE_AST_MATCHES(expr, R"((QualifiedPath
  (PathSegment collections)
  (PathSegment HashMap
    (Type string)
    (Type i32))))");
    }

    SECTION(":: prefixed in function call") {
        auto fixture = createParserFixture("::Vector<i32>()");
        auto *expr = fixture->parseExpression();

        REQUIRE(expr != nullptr);
        REQUIRE_AST_MATCHES(expr, R"((CallExpr
  (QualifiedPath
    (PathSegment Vector
      (Type i32)))))");
    }

    SECTION("Complex :: prefixed expression") {
        auto fixture = createParserFixture("::Map<string, ::Vector<i32>>");
        auto *expr = fixture->parseExpression();

        REQUIRE(expr != nullptr);
        REQUIRE_AST_MATCHES(expr, R"((QualifiedPath
  (PathSegment Map
    (Type string)
    (QualifiedPath
      (PathSegment Vector
        (Type i32))))))");
    }
}

TEST_CASE("Qualified Path Parsing - Context Disambiguation", "[parser][types][qualified-paths][disambiguation]") {
    SECTION("< in expression without :: is comparison") {
        auto fixture = createParserFixture("a < b");
        auto *expr = fixture->parseExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astBinaryExpr);

        REQUIRE_AST_MATCHES(expr, R"((BinaryExpr <
  (Identifier a)
  (Identifier b)))");
    }

    SECTION("Chained comparisons") {
        auto fixture = createParserFixture("a < b < c < d");
        auto *expr = fixture->parseExpression();

        REQUIRE(expr != nullptr);
        // Should parse as left-associative comparisons
        REQUIRE_AST_MATCHES(expr, R"((BinaryExpr <
  (BinaryExpr <
    (BinaryExpr <
      (Identifier a)
      (Identifier b))
    (Identifier c))
  (Identifier d)))");
    }

    SECTION("Mixed :: and comparison") {
        auto fixture = createParserFixture("::create<T>() < threshold");
        auto *expr = fixture->parseExpression();

        REQUIRE(expr != nullptr);
        REQUIRE_AST_MATCHES(expr, R"((BinaryExpr <
  (CallExpr
    (QualifiedPath
      (PathSegment create
        (Identifier T))))
  (Identifier threshold)))");
    }
}

TEST_CASE("Qualified Path Parsing - Error Cases", "[parser][types][qualified-paths][errors]") {
    SECTION("Empty generic arguments") {
        auto fixture = createParserFixture("Vector<>");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Unclosed generic arguments") {
        auto fixture = createParserFixture("Vector<i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing type argument") {
        auto fixture = createParserFixture("Map<string,>");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION(":: without following identifier") {
        auto fixture = createParserFixture("::");
        auto *expr = fixture->parseExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION(":: followed by number") {
        auto fixture = createParserFixture("::123");
        auto *expr = fixture->parseExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Qualified Path Parsing - Integration with Declarations", "[parser][types][qualified-paths][integration]") {
    SECTION("Variable declaration with qualified type") {
        auto fixture = createParserFixture("var items: collections.Vector<string>");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier items)
  (QualifiedPath
    (PathSegment collections)
    (PathSegment Vector
      (Type string)))))");
    }

    SECTION("Function parameter with generic type") {
        auto fixture = createParserFixture("func process(data Map<string, i32>)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->parameters.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier process)
  (FuncParamDeclaration
    (Identifier data)
    (QualifiedPath
      (PathSegment Map
        (Type string)
        (Type i32))))))");
    }

    SECTION("Function return type with qualified path") {
        auto fixture = createParserFixture("func create() graphics.Point<f64>");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier create)
  (QualifiedPath
    (PathSegment graphics)
    (PathSegment Point
      (Type f64)))))");
    }
}
