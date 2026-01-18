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

TEST_CASE("Struct Declaration Parsing - Basic Forms", "[parser][declarations][struct-decl]") {
    SECTION("struct Point {}") {
        auto fixture = createParserFixture("struct Point {}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astStructDeclaration);

        auto *structDecl = static_cast<StructDeclarationNode *>(stmt);
        REQUIRE(structDecl->name != nullptr);
        REQUIRE(structDecl->members.empty());
        REQUIRE(structDecl->annotations.empty());

        REQUIRE_AST_MATCHES(stmt, R"((StructDeclaration
  (Identifier Point)))");
    }

    SECTION("pub struct Point {}") {
        auto fixture = createParserFixture("pub struct Point {}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astStructDeclaration);

        auto *structDecl = static_cast<StructDeclarationNode *>(stmt);
        REQUIRE(structDecl->name != nullptr);
        REQUIRE(structDecl->members.empty());
        REQUIRE((structDecl->flags & flgPublic) != 0);

        REQUIRE_AST_MATCHES(stmt, R"((StructDeclaration
  (Identifier Point)))");
    }

    SECTION("struct Point { x i32 }") {
        auto fixture = createParserFixture("struct Point { x i32 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astStructDeclaration);

        auto *structDecl = static_cast<StructDeclarationNode *>(stmt);
        REQUIRE(structDecl->name != nullptr);
        REQUIRE(structDecl->members.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((StructDeclaration
  (Identifier Point)
  (FieldDeclaration
    (Identifier x)
    (Type i32))))");
    }

    SECTION("struct Point { x i32; y f64 }") {
        auto fixture = createParserFixture("struct Point { x i32; y f64 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astStructDeclaration);

        auto *structDecl = static_cast<StructDeclarationNode *>(stmt);
        REQUIRE(structDecl->name != nullptr);
        REQUIRE(structDecl->members.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((StructDeclaration
  (Identifier Point)
  (FieldDeclaration
    (Identifier x)
    (Type i32))
  (FieldDeclaration
    (Identifier y)
    (Type f64))))");
    }

    SECTION("struct Point { x i32 = 0 }") {
        auto fixture = createParserFixture("struct Point { x i32 = 0 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astStructDeclaration);

        auto *structDecl = static_cast<StructDeclarationNode *>(stmt);
        REQUIRE(structDecl->name != nullptr);
        REQUIRE(structDecl->members.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((StructDeclaration
  (Identifier Point)
  (FieldDeclaration
    (Identifier x)
    (Type i32)
    (Int 0))))");
    }
}

TEST_CASE("Class Declaration Parsing - Basic Forms", "[parser][declarations][class-decl]") {
    SECTION("class Shape {}") {
        auto fixture = createParserFixture("class Shape {}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astClassDeclaration);

        auto *classDecl = static_cast<ClassDeclarationNode *>(stmt);
        REQUIRE(classDecl->name != nullptr);
        REQUIRE(classDecl->members.empty());
        REQUIRE(classDecl->annotations.empty());
        REQUIRE(classDecl->base == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((ClassDeclaration
  (Identifier Shape)))");
    }

    SECTION("pub class Shape {}") {
        auto fixture = createParserFixture("pub class Shape {}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astClassDeclaration);

        auto *classDecl = static_cast<ClassDeclarationNode *>(stmt);
        REQUIRE(classDecl->name != nullptr);
        REQUIRE(classDecl->members.empty());
        REQUIRE((classDecl->flags & flgPublic) != 0);

        REQUIRE_AST_MATCHES(stmt, R"((ClassDeclaration
  (Identifier Shape)))");
    }

    SECTION("class Circle : i32 {}") {
        auto fixture = createParserFixture("class Circle : i32 {}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astClassDeclaration);

        auto *classDecl = static_cast<ClassDeclarationNode *>(stmt);
        REQUIRE(classDecl->name != nullptr);
        REQUIRE(classDecl->base != nullptr);
        REQUIRE(classDecl->members.empty());

        REQUIRE_AST_MATCHES(stmt, R"((ClassDeclaration
  (Identifier Circle)
  (Type i32)))");
    }

    SECTION("class Point { x i32; y f64 }") {
        auto fixture = createParserFixture("class Point { x i32; y f64 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astClassDeclaration);

        auto *classDecl = static_cast<ClassDeclarationNode *>(stmt);
        REQUIRE(classDecl->name != nullptr);
        REQUIRE(classDecl->members.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((ClassDeclaration
  (Identifier Point)
  (FieldDeclaration
    (Identifier x)
    (Type i32))
  (FieldDeclaration
    (Identifier y)
    (Type f64))))");
    }
}

TEST_CASE("Struct/Class with Methods", "[parser][declarations][struct-decl][class-decl][methods]") {
    SECTION("struct Point { func distance() f64 }") {
        auto fixture = createParserFixture("struct Point { func distance() f64 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astStructDeclaration);

        auto *structDecl = static_cast<StructDeclarationNode *>(stmt);
        REQUIRE(structDecl->name != nullptr);
        REQUIRE(structDecl->members.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((StructDeclaration
  (Identifier Point)
  (FuncDeclaration
    (Identifier distance)
    (Type f64))))");
    }

    SECTION("class Shape { func area() f64 }") {
        auto fixture = createParserFixture("class Shape { func area() f64 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astClassDeclaration);

        auto *classDecl = static_cast<ClassDeclarationNode *>(stmt);
        REQUIRE(classDecl->name != nullptr);
        REQUIRE(classDecl->members.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((ClassDeclaration
  (Identifier Shape)
  (FuncDeclaration
    (Identifier area)
    (Type f64))))");
    }
}

TEST_CASE("Member Visibility", "[parser][declarations][struct-decl][class-decl][visibility]") {
    SECTION("struct Point { priv x i32 }") {
        auto fixture = createParserFixture("struct Point { priv x i32 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astStructDeclaration);

        auto *structDecl = static_cast<StructDeclarationNode *>(stmt);
        REQUIRE(structDecl->members.size() == 1);

        auto *member = structDecl->members[0];
        REQUIRE((member->flags & flgPublic) == 0); // private member

        REQUIRE_AST_MATCHES(stmt, R"((StructDeclaration
  (Identifier Point)
  (FieldDeclaration
    (Identifier x)
    (Type i32))))");
    }

    SECTION("struct Point { x i32 }") {
        auto fixture = createParserFixture("struct Point { x i32 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astStructDeclaration);

        auto *structDecl = static_cast<StructDeclarationNode *>(stmt);
        REQUIRE(structDecl->members.size() == 1);

        auto *member = structDecl->members[0];
        REQUIRE((member->flags & flgPublic) != 0); // public by default

        REQUIRE_AST_MATCHES(stmt, R"((StructDeclaration
  (Identifier Point)
  (FieldDeclaration
    (Identifier x)
    (Type i32))))");
    }

    SECTION("class Point { priv func helper() }") {
        auto fixture = createParserFixture("class Point { priv func helper() {} }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astClassDeclaration);

        auto *classDecl = static_cast<ClassDeclarationNode *>(stmt);
        REQUIRE(classDecl->members.size() == 1);

        auto *member = classDecl->members[0];
        REQUIRE((member->flags & flgPublic) == 0); // private method

        REQUIRE_AST_MATCHES(stmt, R"((ClassDeclaration
  (Identifier Point)
  (FuncDeclaration
    (Identifier helper)
    (BlockStmt))))");
    }
}

TEST_CASE("Generic Struct/Class Declarations", "[parser][declarations][struct-decl][class-decl][generics]") {
    SECTION("struct Container<T> {}") {
        auto fixture = createParserFixture("struct Container<T> {}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astGenericDeclaration);

        auto *genericDecl = static_cast<GenericDeclarationNode *>(stmt);
        REQUIRE(genericDecl->parameters.size() == 1);
        REQUIRE(genericDecl->decl != nullptr);
        REQUIRE(genericDecl->decl->kind == astStructDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((GenericDeclaration
  (TypeParameterDeclaration
    (Identifier T))
  (StructDeclaration
    (Identifier Container))))");
    }

    SECTION("class Vector<T, U> {}") {
        auto fixture = createParserFixture("class Vector<T, U> {}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astGenericDeclaration);

        auto *genericDecl = static_cast<GenericDeclarationNode *>(stmt);
        REQUIRE(genericDecl->parameters.size() == 2);
        REQUIRE(genericDecl->decl != nullptr);
        REQUIRE(genericDecl->decl->kind == astClassDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((GenericDeclaration
  (TypeParameterDeclaration
    (Identifier T))
  (TypeParameterDeclaration
    (Identifier U))
  (ClassDeclaration
    (Identifier Vector))))");
    }
}

TEST_CASE("Annotations", "[parser][declarations][struct-decl][class-decl][annotations]") {
    SECTION("struct with single annotation") {
        auto fixture = createParserFixture("struct Vector { `Hello = 20 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astStructDeclaration);

        auto *structDecl = static_cast<StructDeclarationNode *>(stmt);
        REQUIRE(structDecl->name != nullptr);
        REQUIRE(structDecl->members.size() == 0);
        REQUIRE(structDecl->annotations.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((StructDeclaration
  (Identifier Vector)
  (AnnotationList
    (Annotation Hello (Int 20)))))");
    }

    SECTION("struct with multiple annotations") {
        auto fixture = createParserFixture("struct Vector { `Hello = 20 `isVector = true }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astStructDeclaration);

        auto *structDecl = static_cast<StructDeclarationNode *>(stmt);
        REQUIRE(structDecl->members.size() == 0);
        REQUIRE(structDecl->annotations.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((StructDeclaration
  (Identifier Vector)
  (AnnotationList
    (Annotation Hello (Int 20))
    (Annotation isVector (Bool true)))))");
    }

    SECTION("class with annotation and field") {
        auto fixture = createParserFixture("class Shape { `serializable = true area f64 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astClassDeclaration);

        auto *classDecl = static_cast<ClassDeclarationNode *>(stmt);
        REQUIRE(classDecl->annotations.size() == 1);
        REQUIRE(classDecl->members.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((ClassDeclaration
  (Identifier Shape)
  (AnnotationList
    (Annotation serializable (Bool true)))
  (FieldDeclaration
    (Identifier area)
    (Type f64))))");
    }
}

TEST_CASE("Error Cases", "[parser][declarations][struct-decl][class-decl][errors]") {
    SECTION("extern struct Point {} - should fail") {
        auto fixture = createParserFixture("extern struct Point {}");
        auto *stmt = fixture->parseDeclaration();
        expectParseFailure(stmt);
    }

    SECTION("extern class Shape {} - should fail") {
        auto fixture = createParserFixture("extern class Shape {}");
        auto *stmt = fixture->parseDeclaration();
        expectParseFailure(stmt);
    }

    SECTION("struct without name - should fail") {
        auto fixture = createParserFixture("struct {}");
        auto *stmt = fixture->parseDeclaration();
        expectParseFailure(stmt);
    }

    SECTION("class without name - should fail") {
        auto fixture = createParserFixture("class {}");
        auto *stmt = fixture->parseDeclaration();
        expectParseFailure(stmt);
    }

    SECTION("struct without opening brace - should fail") {
        auto fixture = createParserFixture("struct Point");
        auto *stmt = fixture->parseDeclaration();
        expectParseFailure(stmt);
    }

    SECTION("struct without closing brace - should fail") {
        auto fixture = createParserFixture("struct Point {");
        auto *stmt = fixture->parseDeclaration();
        expectParseFailure(stmt);
    }
}
