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

TEST_CASE("Function Declaration Parsing - Basic Forms", "[parser][declarations][func-decl]") {
    SECTION("func add") {
        auto fixture = createParserFixture("func add");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->name != nullptr);
        REQUIRE(funcDecl->parameters.empty());
        REQUIRE(funcDecl->returnType == nullptr);
        REQUIRE(funcDecl->body == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier add)))");
    }

    SECTION("func add()") {
        auto fixture = createParserFixture("func add()");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->name != nullptr);
        REQUIRE(funcDecl->parameters.empty());
        REQUIRE(funcDecl->returnType == nullptr);
        REQUIRE(funcDecl->body == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier add)))");
    }

    SECTION("func say() {}") {
        auto fixture = createParserFixture("func say() {}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->name != nullptr);
        REQUIRE(funcDecl->parameters.empty());
        REQUIRE(funcDecl->returnType == nullptr);
        REQUIRE(funcDecl->body != nullptr);
        REQUIRE(funcDecl->body->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier say)
  (BlockStmt)))");
    }
}

TEST_CASE("Function Declaration Parsing - Parameters", "[parser][declarations][func-decl][params]") {
    SECTION("func add(a i32)") {
        auto fixture = createParserFixture("func add(a i32)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->name != nullptr);
        REQUIRE(funcDecl->parameters.size() == 1);
        REQUIRE(funcDecl->returnType == nullptr);
        REQUIRE(funcDecl->body == nullptr);

        auto *param = funcDecl->parameters[0];
        REQUIRE(param->kind == astFuncParamDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier add)
  (FuncParamDeclaration
    (Identifier a)
    (Type i32))))");
    }

    SECTION("func add(a i32, b i32)") {
        auto fixture = createParserFixture("func add(a i32, b i32)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->parameters.size() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier add)
  (FuncParamDeclaration
    (Identifier a)
    (Type i32))
  (FuncParamDeclaration
    (Identifier b)
    (Type i32))))");
    }

    SECTION("func add(a i32, b i32 = 5)") {
        auto fixture = createParserFixture("func add(a i32, b i32 = 5)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->parameters.size() == 2);

        auto *param2 = static_cast<FuncParamDeclarationNode *>(funcDecl->parameters[1]);
        REQUIRE(param2->defaultValue != nullptr);
        REQUIRE(param2->defaultValue->kind == astInt);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier add)
  (FuncParamDeclaration
    (Identifier a)
    (Type i32))
  (FuncParamDeclaration
    (Identifier b)
    (Type i32)
    (Int 5))))");
    }

    SECTION("func greet(name string = \"World\")") {
        auto fixture = createParserFixture("func greet(name string = \"World\")");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->parameters.size() == 1);

        auto *param = static_cast<FuncParamDeclarationNode *>(funcDecl->parameters[0]);
        REQUIRE(param->defaultValue != nullptr);
        REQUIRE(param->defaultValue->kind == astString);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier greet)
  (FuncParamDeclaration
    (Identifier name)
    (Type string)
    (String "World"))))");
    }
}

TEST_CASE("Function Declaration Parsing - Return Types", "[parser][declarations][func-decl][return-type]") {
    SECTION("func compute() i32") {
        auto fixture = createParserFixture("func compute() i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->returnType != nullptr);
        REQUIRE(funcDecl->returnType->kind == astPrimitiveType);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier compute)
  (Type i32)))");
    }

    SECTION("func compute() i32") {
        auto fixture = createParserFixture("func compute() i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->returnType != nullptr);
        REQUIRE(funcDecl->returnType->kind == astPrimitiveType);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier compute)
  (Type i32)))");
    }

    SECTION("func println(...args auto) void") {
        auto fixture = createParserFixture("func println(...args auto) void");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->parameters.size() == 1);
        REQUIRE(funcDecl->returnType != nullptr);
        REQUIRE(funcDecl->returnType->kind == astPrimitiveType);

        // Check variadic parameter
        auto *param = static_cast<FuncParamDeclarationNode *>(funcDecl->parameters[0]);
        // Note: variadic handling will be added later

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier println)
  (FuncParamDeclaration
    (Identifier args)
    (Type auto))
  (Type void)))");
    }
}

TEST_CASE("Function Declaration Parsing - Operator Overloads", "[parser][declarations][func-decl][operators]") {
    SECTION("func `+`(other i32) i32") {
        auto fixture = createParserFixture("func `+`(other i32) i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->name != nullptr);
        REQUIRE(funcDecl->isOperatorOverload() == true);
        REQUIRE(funcDecl->operatorToken == TokenKind::Plus);
        REQUIRE(funcDecl->parameters.size() == 1);
        REQUIRE(funcDecl->returnType != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier add)
  (FuncParamDeclaration
    (Identifier other)
    (Type i32))
  (Type i32)))");
    }

    SECTION("func `*`(rhs i32) f64") {
        auto fixture = createParserFixture("func `*`(rhs i32) f64");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->isOperatorOverload() == true);
        REQUIRE(funcDecl->operatorToken == TokenKind::Mult);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier mul)
  (FuncParamDeclaration
    (Identifier rhs)
    (Type i32))
  (Type f64)))");
    }

    SECTION("func `==`(other i32) bool") {
        auto fixture = createParserFixture("func `==`(other i32) bool");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->isOperatorOverload() == true);
        REQUIRE(funcDecl->operatorToken == TokenKind::Equal);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier eq)
  (FuncParamDeclaration
    (Identifier other)
    (Type i32))
  (Type bool)))");
    }

    SECTION("func `[]`(index i32) f64") {
        auto fixture = createParserFixture("func `[]`(index i32) f64");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->isOperatorOverload() == true);
        REQUIRE(funcDecl->operatorToken == TokenKind::IndexOverride);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier index)
  (FuncParamDeclaration
    (Identifier index)
    (Type i32))
  (Type f64)))");
    }

    SECTION("func `()`() i32") {
        auto fixture = createParserFixture("func `()`() i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->isOperatorOverload() == true);
        REQUIRE(funcDecl->operatorToken == TokenKind::CallOverride);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier call)
  (Type i32)))");
    }

    SECTION("func `[]`() bool => true") {
        auto fixture = createParserFixture("func `[]`() bool => true");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->isOperatorOverload() == true);
        REQUIRE(funcDecl->operatorToken == TokenKind::IndexOverride);
        REQUIRE(funcDecl->body != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier index)
  (Type bool)
  (Bool true)))");
    }
}

TEST_CASE("Function Declaration Parsing - Increment/Decrement Operator Overloads", "[parser][declarations][func-decl][inc-dec-operators]") {
    SECTION("func `++`() auto") {
        auto fixture = createParserFixture("func `++`() auto");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->isOperatorOverload() == true);
        REQUIRE(funcDecl->operatorToken == TokenKind::PlusPlus);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier inc)
  (Type auto)))");
    }

    SECTION("func `--`() auto") {
        auto fixture = createParserFixture("func `--`() auto");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->isOperatorOverload() == true);
        REQUIRE(funcDecl->operatorToken == TokenKind::MinusMinus);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier dec)
  (Type auto)))");
    }
}

TEST_CASE("Function Declaration Parsing - Invalid Operator Overloads", "[parser][declarations][func-decl][invalid-operators]") {
    SECTION("func `!`() bool - logical not not allowed") {
        auto fixture = createParserFixture("func `!`() bool");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("func `&`() auto - address-of not allowed") {
        auto fixture = createParserFixture("func `&`() auto");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("func `^`() auto - bitwise not as unary not allowed") {
        auto fixture = createParserFixture("func `^`() auto");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("func `~`() auto - bitwise complement not allowed") {
        auto fixture = createParserFixture("func `~`() auto");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("func `as`() auto - cast operator not allowed") {
        auto fixture = createParserFixture("func `as`() auto");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Function Declaration Parsing - Binary Use of Restricted Operators", "[parser][declarations][func-decl][binary-restricted]") {
    SECTION("func `&`(other i32) i32 - binary AND allowed") {
        auto fixture = createParserFixture("func `&`(other i32) i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->isOperatorOverload() == true);
        REQUIRE(funcDecl->operatorToken == TokenKind::BAnd);
        REQUIRE(funcDecl->parameters.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier band)
  (FuncParamDeclaration
    (Identifier other)
    (Type i32))
  (Type i32)))");
    }

    SECTION("func `^`(other i32) i32 - binary XOR allowed") {
        auto fixture = createParserFixture("func `^`(other i32) i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->isOperatorOverload() == true);
        REQUIRE(funcDecl->operatorToken == TokenKind::BXor);
        REQUIRE(funcDecl->parameters.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier bxor)
  (FuncParamDeclaration
    (Identifier other)
    (Type i32))
  (Type i32)))");
    }

    SECTION("func `&&`(other bool) bool - binary logical AND allowed") {
        auto fixture = createParserFixture("func `&&`(other bool) bool");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->isOperatorOverload() == true);
        REQUIRE(funcDecl->operatorToken == TokenKind::LAnd);
        REQUIRE(funcDecl->parameters.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier land)
  (FuncParamDeclaration
    (Identifier other)
    (Type bool))
  (Type bool)))");
    }
}

TEST_CASE("Function Declaration Parsing - Expression Bodies", "[parser][declarations][func-decl][expr-body]") {
    SECTION("func add(a i32, b i32) => a + b") {
        auto fixture = createParserFixture("func add(a i32, b i32) => a + b");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->body != nullptr);
        REQUIRE(funcDecl->body->kind == astBinaryExpr);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier add)
  (FuncParamDeclaration
    (Identifier a)
    (Type i32))
  (FuncParamDeclaration
    (Identifier b)
    (Type i32))
  (BinaryExpr + (Identifier a) (Identifier b))))");
    }

    SECTION("func multiply(x i32, y i32) i32 => x * y") {
        auto fixture = createParserFixture("func multiply(x i32, y i32) i32 => x * y");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->returnType != nullptr);
        REQUIRE(funcDecl->body != nullptr);
        REQUIRE(funcDecl->body->kind == astBinaryExpr);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier multiply)
  (FuncParamDeclaration
    (Identifier x)
    (Type i32))
  (FuncParamDeclaration
    (Identifier y)
    (Type i32))
  (Type i32)
  (BinaryExpr * (Identifier x) (Identifier y))))");
    }

    SECTION("func getValue() => 42") {
        auto fixture = createParserFixture("func getValue() => 42");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->body != nullptr);
        REQUIRE(funcDecl->body->kind == astInt);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier getValue)
  (Int 42)))");
    }
}

TEST_CASE("Function Declaration Parsing - Block Bodies", "[parser][declarations][func-decl][block-body]") {
    SECTION("func compute() i32 { return 100 * global }") {
        auto fixture = createParserFixture("func compute() i32 { return 100 * global }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->returnType != nullptr);
        REQUIRE(funcDecl->body != nullptr);
        REQUIRE(funcDecl->body->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier compute)
  (Type i32)
  (BlockStmt
    (ReturnStmt
      (BinaryExpr * (Int 100) (Identifier global))))))");
    }

    SECTION("func initialize() { setupGlobals() }") {
        auto fixture = createParserFixture("func initialize() { setupGlobals() }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->body != nullptr);
        REQUIRE(funcDecl->body->kind == astBlockStmt);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier initialize)
  (BlockStmt
    (ExprStmt
      (CallExpr
        (Identifier setupGlobals))))))");
    }
}

TEST_CASE("Function Declaration Parsing - With Attributes", "[parser][declarations][func-decl][attributes]") {
    SECTION("@virtual func compute() i32") {
        auto fixture = createParserFixture("@virtual func compute() i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);
        REQUIRE(stmt->hasAttributes());
        REQUIRE(stmt->getAttributeCount() == 1);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->returnType != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier compute)
  (Type i32)))");
    }

    SECTION("@inline @deprecated(\"Use newFunc instead\") func oldFunc()") {
        auto fixture = createParserFixture("@inline @deprecated(\"Use newFunc instead\") func oldFunc()");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);
        REQUIRE(stmt->hasAttributes());
        REQUIRE(stmt->getAttributeCount() == 2);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier oldFunc)))");
    }
}

TEST_CASE("Function Declaration Parsing - Error Cases", "[parser][declarations][func-decl][errors]") {
    SECTION("func without name") {
        auto fixture = createParserFixture("func");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("func with missing parameter type") {
        auto fixture = createParserFixture("func test(a)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("func with unclosed parameter list") {
        auto fixture = createParserFixture("func test(a: i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("func with invalid parameter syntax") {
        auto fixture = createParserFixture("func test(: i32)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Function Declaration Parsing - Complex Cases", "[parser][declarations][func-decl][complex]") {
    SECTION("func calculate(x i32, y f64 = 3.14, enabled bool = true)") {
        auto fixture = createParserFixture("func calculate(x i32, y f64 = 3.14, enabled bool = true)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->parameters.size() == 3);

        // First parameter should have no default
        auto *param1 = static_cast<FuncParamDeclarationNode *>(funcDecl->parameters[0]);
        REQUIRE(param1->defaultValue == nullptr);

        // Second parameter should have f64 default
        auto *param2 = static_cast<FuncParamDeclarationNode *>(funcDecl->parameters[1]);
        REQUIRE(param2->defaultValue != nullptr);
        REQUIRE(param2->defaultValue->kind == astFloat);

        // Third parameter should have bool default
        auto *param3 = static_cast<FuncParamDeclarationNode *>(funcDecl->parameters[2]);
        REQUIRE(param3->defaultValue != nullptr);
        REQUIRE(param3->defaultValue->kind == astBool);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier calculate)
  (FuncParamDeclaration
    (Identifier x)
    (Type i32))
  (FuncParamDeclaration
    (Identifier y)
    (Type f64)
    (Float 3.14))
  (FuncParamDeclaration
    (Identifier enabled)
    (Type bool)
    (Bool true))))");
    }
}

TEST_CASE("Function Declaration Parsing - Generic Functions", "[parser][declarations][func-decl][generics]") {
    SECTION("func a<T>(){}") {
        auto fixture = createParserFixture("func a<T>(){}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astGenericDeclaration);

        auto *genericDecl = static_cast<GenericDeclarationNode *>(stmt);
        REQUIRE(genericDecl->parameters.size() == 1);
        REQUIRE(genericDecl->decl != nullptr);
        REQUIRE(genericDecl->decl->kind == astFuncDeclaration);

        // Check the generic parameter
        auto *param = static_cast<TypeParameterDeclarationNode *>(genericDecl->parameters[0]);
        REQUIRE(param->name != nullptr);
        REQUIRE(param->constraint == nullptr);
        REQUIRE(param->defaultValue == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((GenericDeclaration
  (TypeParameterDeclaration
    (Identifier T))
  (FuncDeclaration
    (Identifier a)
    (BlockStmt))))");
    }

    SECTION("func a<T:i32>{}") {
        auto fixture = createParserFixture("func a<T:i32>{}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astGenericDeclaration);

        auto *genericDecl = static_cast<GenericDeclarationNode *>(stmt);
        REQUIRE(genericDecl->parameters.size() == 1);

        // Check the generic parameter has constraint
        auto *param = static_cast<TypeParameterDeclarationNode *>(genericDecl->parameters[0]);
        REQUIRE(param->name != nullptr);
        REQUIRE(param->constraint != nullptr);
        REQUIRE(param->defaultValue == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((GenericDeclaration
  (TypeParameterDeclaration
    (Identifier T)
    (Type i32))
  (FuncDeclaration
    (Identifier a)
    (BlockStmt))))");
    }

    SECTION("func a<X, Y=i32>(){}") {
        auto fixture = createParserFixture("func a<X, Y=i32>(){}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astGenericDeclaration);

        auto *genericDecl = static_cast<GenericDeclarationNode *>(stmt);
        REQUIRE(genericDecl->parameters.size() == 2);

        // Check first parameter (no constraint, no default)
        auto *param1 = static_cast<TypeParameterDeclarationNode *>(genericDecl->parameters[0]);
        REQUIRE(param1->name != nullptr);
        REQUIRE(param1->constraint == nullptr);
        REQUIRE(param1->defaultValue == nullptr);

        // Check second parameter (has default)
        auto *param2 = static_cast<TypeParameterDeclarationNode *>(genericDecl->parameters[1]);
        REQUIRE(param2->name != nullptr);
        REQUIRE(param2->constraint == nullptr);
        REQUIRE(param2->defaultValue != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((GenericDeclaration
  (TypeParameterDeclaration
    (Identifier X))
  (TypeParameterDeclaration
    (Identifier Y)
    (Type i32))
  (FuncDeclaration
    (Identifier a)
    (BlockStmt))))");
    }

    SECTION("func a<...V:i32>(){}") {
        auto fixture = createParserFixture("func a<...V:i32>(){}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astGenericDeclaration);

        auto *genericDecl = static_cast<GenericDeclarationNode *>(stmt);
        REQUIRE(genericDecl->parameters.size() == 1);

        // Check variadic parameter
        auto *param = static_cast<TypeParameterDeclarationNode *>(genericDecl->parameters[0]);
        REQUIRE(param->name != nullptr);
        REQUIRE(param->constraint != nullptr);
        REQUIRE(param->defaultValue == nullptr);
        REQUIRE((param->flags & flgVariadic) != 0);

        REQUIRE_AST_MATCHES(stmt, R"((GenericDeclaration
  (TypeParameterDeclaration
    (Identifier V)
    (Type i32))
  (FuncDeclaration
    (Identifier a)
    (BlockStmt))))");
    }

    SECTION("func max<T>(a i32, b i32) i32 => a") {
        auto fixture = createParserFixture("func max<T>(a i32, b i32) i32 => a");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astGenericDeclaration);

        auto *genericDecl = static_cast<GenericDeclarationNode *>(stmt);
        REQUIRE(genericDecl->parameters.size() == 1);
        REQUIRE(genericDecl->decl != nullptr);
        REQUIRE(genericDecl->decl->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(genericDecl->decl);
        REQUIRE(funcDecl->parameters.size() == 2);
        REQUIRE(funcDecl->returnType != nullptr);
        REQUIRE(funcDecl->body != nullptr);
        REQUIRE(funcDecl->body->kind == astIdentifier);

        REQUIRE_AST_MATCHES(stmt, R"((GenericDeclaration
  (TypeParameterDeclaration
    (Identifier T))
  (FuncDeclaration
    (Identifier max)
    (FuncParamDeclaration
      (Identifier a)
      (Type i32))
    (FuncParamDeclaration
      (Identifier b)
      (Type i32))
    (Type i32)
    (Identifier a))))");
    }

    SECTION("func process<T:i32, U=bool>(data i32, options bool) i32") {
        auto fixture = createParserFixture("func process<T:i32, U=bool>(data i32, options bool) i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astGenericDeclaration);

        auto *genericDecl = static_cast<GenericDeclarationNode *>(stmt);
        REQUIRE(genericDecl->parameters.size() == 2);

        // Check first parameter has constraint
        auto *param1 = static_cast<TypeParameterDeclarationNode *>(genericDecl->parameters[0]);
        REQUIRE(param1->constraint != nullptr);
        REQUIRE(param1->defaultValue == nullptr);

        // Check second parameter has default
        auto *param2 = static_cast<TypeParameterDeclarationNode *>(genericDecl->parameters[1]);
        REQUIRE(param2->constraint == nullptr);
        REQUIRE(param2->defaultValue != nullptr);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(genericDecl->decl);
        REQUIRE(funcDecl->parameters.size() == 2);
        REQUIRE(funcDecl->returnType != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((GenericDeclaration
  (TypeParameterDeclaration
    (Identifier T)
    (Type i32))
  (TypeParameterDeclaration
    (Identifier U)
    (Type bool))
  (FuncDeclaration
    (Identifier process)
    (FuncParamDeclaration
      (Identifier data)
      (Type i32))
    (FuncParamDeclaration
      (Identifier options)
      (Type bool))
    (Type i32))))");
    }
}

TEST_CASE("Function Declaration Parsing - Generic Functions Error Cases", "[parser][declarations][func-decl][generics][errors]") {
    SECTION("Generic parameter ordering - defaulted before non-defaulted") {
        auto fixture = createParserFixture("func test<T=i32, U>(){}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Generic parameter ordering - variadic not last") {
        auto fixture = createParserFixture("func test<...T, U>(){}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Generic parameter ordering - variadic with multiple") {
        auto fixture = createParserFixture("func test<T, ...U, V>(){}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Unclosed generic parameter list") {
        auto fixture = createParserFixture("func test<T");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Empty generic parameter list") {
        auto fixture = createParserFixture("func test<>(){}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Invalid generic parameter syntax") {
        auto fixture = createParserFixture("func test<:Constraint>(){}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Function Declaration Parsing - Visibility Modifiers", "[parser][declarations][func-decl][visibility]") {
    SECTION("pub func calculate() i32") {
        auto fixture = createParserFixture("pub func calculate() i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);
        REQUIRE((stmt->flags & flgPublic) != 0);
        REQUIRE((stmt->flags & flgExtern) == 0);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->name != nullptr);
        REQUIRE(funcDecl->returnType != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier calculate)
  (Type i32)))");
    }

    SECTION("extern func printf(fmt string, ...args auto) void") {
        auto fixture = createParserFixture("extern func printf(fmt string, ...args auto) void");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);
        REQUIRE((stmt->flags & flgExtern) != 0);
        REQUIRE((stmt->flags & flgPublic) == 0);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->name != nullptr);
        REQUIRE(funcDecl->parameters.size() == 2);
        REQUIRE(funcDecl->returnType != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier printf)
  (FuncParamDeclaration
    (Identifier fmt)
    (Type string))
  (FuncParamDeclaration
    (Identifier args)
    (Type auto))
  (Type void)))");
    }

    SECTION("@inline pub func fastOp(x i32) i32 => x * 2") {
        auto fixture = createParserFixture("@inline pub func fastOp(x i32) i32 => x * 2");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);
        REQUIRE((stmt->flags & flgPublic) != 0);
        REQUIRE(stmt->hasAttributes());
        REQUIRE(stmt->getAttributeCount() == 1);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(stmt);
        REQUIRE(funcDecl->name != nullptr);
        REQUIRE(funcDecl->parameters.size() == 1);
        REQUIRE(funcDecl->returnType != nullptr);
        REQUIRE(funcDecl->body != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
  (Identifier fastOp)
  (FuncParamDeclaration
    (Identifier x)
    (Type i32))
  (Type i32)
  (BinaryExpr * (Identifier x) (Int 2))))");
    }

    SECTION("pub func max<T>(a i32, b i32) i32 => a") {
        auto fixture = createParserFixture("pub func max<T>(a i32, b i32) i32 => a");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astGenericDeclaration);
        REQUIRE((stmt->flags & flgPublic) != 0);

        auto *genericDecl = static_cast<GenericDeclarationNode *>(stmt);
        REQUIRE(genericDecl->parameters.size() == 1);
        REQUIRE(genericDecl->decl != nullptr);
        REQUIRE(genericDecl->decl->kind == astFuncDeclaration);

        auto *funcDecl = static_cast<FuncDeclarationNode *>(genericDecl->decl);
        REQUIRE(funcDecl->parameters.size() == 2);
        REQUIRE(funcDecl->returnType != nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((GenericDeclaration
  (TypeParameterDeclaration
    (Identifier T))
  (FuncDeclaration
    (Identifier max)
    (FuncParamDeclaration
      (Identifier a)
      (Type i32))
    (FuncParamDeclaration
      (Identifier b)
      (Type i32))
    (Type i32)
    (Identifier a))))");
    }
}

TEST_CASE("Function Declaration Parsing - Extern Validation Errors", "[parser][declarations][func-decl][extern][errors]") {
    SECTION("extern func with generic parameters") {
        auto fixture = createParserFixture("extern func process<T>(data T) i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("extern func without return type") {
        auto fixture = createParserFixture("extern func calculate(x i32)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("extern func with expression body") {
        auto fixture = createParserFixture("extern func add(a i32, b i32) i32 => a + b");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("extern func with block body") {
        auto fixture = createParserFixture("extern func multiply(a i32, b i32) i32 { return a * b }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("extern enum declaration") {
        auto fixture = createParserFixture("extern enum Status { Ok, Error }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}
