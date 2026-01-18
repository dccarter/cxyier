#include "catch2.hpp"
#include "../../parser_test_utils.hpp"
#include "cxy/ast/declarations.hpp"
#include "cxy/ast/identifiers.hpp"
#include "cxy/ast/types.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::test;

TEST_CASE("Type Declaration Parsing - Basic Forms", "[parser][declarations][type-decl]") {
    SECTION("type Number = i32") {
        auto fixture = createParserFixture("type Number = i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        auto *typeDecl = static_cast<TypeDeclarationNode *>(stmt);
        REQUIRE(typeDecl->name != nullptr);
        REQUIRE(typeDecl->type != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Number)
  (Type i32)))");
    }

    SECTION("type Bool = bool") {
        auto fixture = createParserFixture("type Bool = bool");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        auto *typeDecl = static_cast<TypeDeclarationNode *>(stmt);
        REQUIRE(typeDecl->name != nullptr);
        REQUIRE(typeDecl->type != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Bool)
  (Type bool)))");
    }

    SECTION("type Str = string") {
        auto fixture = createParserFixture("type Str = string");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Str)
  (Type string)))");
    }
}

TEST_CASE("Type Declaration Parsing - Union Types", "[parser][declarations][type-decl]") {
    SECTION("type Number = i32 | u32") {
        auto fixture = createParserFixture("type Number = i32 | u32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        auto *typeDecl = static_cast<TypeDeclarationNode *>(stmt);
        REQUIRE(typeDecl->name != nullptr);
        REQUIRE(typeDecl->type != nullptr);
        REQUIRE(typeDecl->type->kind == astUnionType);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Number)
  (UnionType
    (Type i32)
    (Type u32))))");
    }

    SECTION("type Value = i32 | f64 | string") {
        auto fixture = createParserFixture("type Value = i32 | f64 | string");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Value)
  (UnionType
    (Type i32)
    (Type f64)
    (Type string))))");
    }

    SECTION("type Optional = i32 | string") {
        auto fixture = createParserFixture("type Optional = i32 | string");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Optional)
  (UnionType
    (Type i32)
    (Type string))))");
    }
}

TEST_CASE("Type Declaration Parsing - Function Types", "[parser][declarations][type-decl]") {
    SECTION("type Func = func() -> void") {
        auto fixture = createParserFixture("type Func = func() -> void");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        auto *typeDecl = static_cast<TypeDeclarationNode *>(stmt);
        REQUIRE(typeDecl->name != nullptr);
        REQUIRE(typeDecl->type != nullptr);
        REQUIRE(typeDecl->type->kind == astFunctionType);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Func)
  (FunctionType
    (Type void))))");
    }

    SECTION("type Handler = func(i32) -> string") {
        auto fixture = createParserFixture("type Handler = func(i32) -> string");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Handler)
  (FunctionType
    (Type i32)
    (Type string))))");
    }

    SECTION("type Processor = func(i32, string) -> bool") {
        auto fixture = createParserFixture("type Processor = func(i32, string) -> bool");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Processor)
  (FunctionType
    (Type i32)
    (Type string)
    (Type bool))))");
    }

    SECTION("type Callback = func() -> i32") {
        auto fixture = createParserFixture("type Callback = func() -> i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Callback)
  (FunctionType
    (Type i32))))");
    }
}

TEST_CASE("Type Declaration Parsing - Complex Types", "[parser][declarations][type-decl]") {
    SECTION("type Custom = (i32, string)") {
        auto fixture = createParserFixture("type Custom = (i32, string)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        auto *typeDecl = static_cast<TypeDeclarationNode *>(stmt);
        REQUIRE(typeDecl->name != nullptr);
        REQUIRE(typeDecl->type != nullptr);
        REQUIRE(typeDecl->type->kind == astTupleType);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Custom)
  (TupleType
    (Type i32)
    (Type string))))");
    }

    SECTION("type Array = [10]i32") {
        auto fixture = createParserFixture("type Array = [10]i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        auto *typeDecl = static_cast<TypeDeclarationNode *>(stmt);
        REQUIRE(typeDecl->type != nullptr);
        REQUIRE(typeDecl->type->kind == astArrayType);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Array)
  (ArrayType
    (Int 10)
    (Type i32))))");
    }

    SECTION("type Dynamic = []string") {
        auto fixture = createParserFixture("type Dynamic = []string");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Dynamic)
  (ArrayType
    (Type string))))");
    }

    SECTION("type Ptr = *i32") {
        auto fixture = createParserFixture("type Ptr = *i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Ptr)
  (PointerType
    (Type i32))))");
    }

    SECTION("type Ref = &string") {
        auto fixture = createParserFixture("type Ref = &string");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Ref)
  (ReferenceType
    (Type string))))");
    }

    SECTION("type Option = ?i32") {
        auto fixture = createParserFixture("type Option = ?i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Option)
  (OptionalType
    (Type i32))))");
    }

    SECTION("type Result = !string") {
        auto fixture = createParserFixture("type Result = !string");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Result)
  (ResultType
    (Type string))))");
    }
}

TEST_CASE("Type Declaration Parsing - Generic Types", "[parser][declarations][type-decl]") {
    SECTION("type Custom<T> = (T, i32)") {
        auto fixture = createParserFixture("type Custom<T> = (T, i32)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astGenericDeclaration);

        auto *genericDecl = static_cast<GenericDeclarationNode *>(stmt);
        REQUIRE(genericDecl->decl != nullptr);
        REQUIRE(genericDecl->decl->kind == astTypeDeclaration);
        REQUIRE(genericDecl->parameters.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((GenericDeclaration
  (TypeParameterDeclaration
    (Identifier T))
  (TypeDeclaration
    (Identifier Custom)
    (TupleType
      (Identifier T)
      (Type i32)))))");
    }

    SECTION("type Container<T, U> = (T, U)") {
        auto fixture = createParserFixture("type Container<T, U> = (T, U)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astGenericDeclaration);

        auto *genericDecl = static_cast<GenericDeclarationNode *>(stmt);
        REQUIRE(genericDecl->parameters.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((GenericDeclaration
  (TypeParameterDeclaration
    (Identifier T))
  (TypeParameterDeclaration
    (Identifier U))
  (TypeDeclaration
    (Identifier Container)
    (TupleType
      (Identifier T)
      (Identifier U)))))");
    }

    SECTION("type Handler<T> = func(T) -> T") {
        auto fixture = createParserFixture("type Handler<T> = func(T) -> T");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astGenericDeclaration);

        auto *genericDecl = static_cast<GenericDeclarationNode *>(stmt);
        REQUIRE(genericDecl->parameters.size() == 1);
        REQUIRE(genericDecl->decl->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((GenericDeclaration
  (TypeParameterDeclaration
    (Identifier T))
  (TypeDeclaration
    (Identifier Handler)
    (FunctionType
      (Identifier T)
      (Identifier T)))))");
    }
}

TEST_CASE("Type Declaration Parsing - Public Types", "[parser][declarations][type-decl]") {
    SECTION("pub type Number = i32") {
        auto fixture = createParserFixture("pub type Number = i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        auto *typeDecl = static_cast<TypeDeclarationNode *>(stmt);
        REQUIRE((typeDecl->flags & flgPublic) != 0);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Number)
  (Type i32)))");
    }

    SECTION("pub type Result<T> = T | Error") {
        auto fixture = createParserFixture("pub type Result<T> = T | Error");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astGenericDeclaration);

        auto *genericDecl = static_cast<GenericDeclarationNode *>(stmt);
        REQUIRE(genericDecl->decl != nullptr);
        REQUIRE(genericDecl->decl->kind == astTypeDeclaration);
        REQUIRE((genericDecl->flags & flgPublic) != 0);

        REQUIRE_AST_MATCHES(stmt, R"((GenericDeclaration
  (TypeParameterDeclaration
    (Identifier T))
  (TypeDeclaration
    (Identifier Result)
    (UnionType
      (Identifier T)
      (Identifier Error)))))");
    }
}

TEST_CASE("Type Declaration Parsing - Complex Nested Types", "[parser][declarations][type-decl]") {
    SECTION("type Complex = func([10]i32) -> ?string") {
        auto fixture = createParserFixture("type Complex = func([10]i32) -> ?string");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Complex)
  (FunctionType
    (ArrayType
    (Int 10)
    (Type i32))
    (OptionalType
      (Type string)))))");
    }

    SECTION("type Nested = (func(i32) -> string, []bool)") {
        auto fixture = createParserFixture("type Nested = (func(i32) -> string, []bool)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Nested)
  (TupleType
    (FunctionType
      (Type i32)
      (Type string))
    (ArrayType
      (Type bool)))))");
    }

    SECTION("type Union = ?i32 | !string") {
        auto fixture = createParserFixture("type Union = ?i32 | !string");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Union)
  (UnionType
    (OptionalType
      (Type i32))
    (ResultType
      (Type string)))))");
    }
}

TEST_CASE("Type Declaration Parsing - Error Cases", "[parser][declarations][type-decl]") {
    SECTION("Missing type name") {
        auto fixture = createParserFixture("type = i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing equals sign") {
        auto fixture = createParserFixture("type Number i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing type expression") {
        auto fixture = createParserFixture("type Number =");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Invalid type expression") {
        auto fixture = createParserFixture("type Number = +");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Generic parameter without name") {
        auto fixture = createParserFixture("type Custom<> = i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Malformed function type") {
        auto fixture = createParserFixture("type Func = func(");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Type Declaration Parsing - Edge Cases", "[parser][declarations][type-decl]") {
    SECTION("Empty tuple type") {
        auto fixture = createParserFixture("type Unit = ()");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Unit)
  (TupleType)))");
    }


    SECTION("Parenthesized type expression") {
        auto fixture = createParserFixture("type Grouped = (i32)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astTypeDeclaration);

        // Should parse as a parenthesized primitive type, not a tuple
        REQUIRE_AST_MATCHES(stmt, R"((TypeDeclaration
  (Identifier Grouped)
  (Type i32)))");
    }
}
