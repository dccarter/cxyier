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

TEST_CASE("Array Type Parsing - Fixed Size Arrays", "[parser][types][complex][array]") {
    SECTION("Simple fixed-size array") {
        auto fixture = createParserFixture("[10]i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astArrayType);

        REQUIRE_AST_MATCHES(expr, R"((ArrayType (Int 10) (Type i32)))");
    }

    SECTION("Array of strings") {
        auto fixture = createParserFixture("[5]string");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astArrayType);

        REQUIRE_AST_MATCHES(expr, R"((ArrayType (Int 5) (Type string)))");
    }

    SECTION("Array with variable size") {
        auto fixture = createParserFixture("[N]i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astArrayType);

        REQUIRE_AST_MATCHES(expr, R"((ArrayType (Identifier N) (Type i32)))");
    }

    SECTION("Array with expression size") {
        auto fixture = createParserFixture("[SIZE + 1]f64");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astArrayType);

        REQUIRE_AST_MATCHES(expr, R"((ArrayType
          (BinaryExpr +
            (Identifier SIZE)
            (Int 1))
          (Type f64)))");
    }
}

TEST_CASE("Array Type Parsing - Dynamic Arrays", "[parser][types][complex][array]") {
    SECTION("Simple dynamic array") {
        auto fixture = createParserFixture("[]i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astArrayType);

        REQUIRE_AST_MATCHES(expr, R"((ArrayType (Type i32)))");
    }

    SECTION("Dynamic array of strings") {
        auto fixture = createParserFixture("[]string");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astArrayType);

        REQUIRE_AST_MATCHES(expr, R"((ArrayType (Type string)))");
    }

    SECTION("Dynamic array of qualified types") {
        auto fixture = createParserFixture("[]mod.Type");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astArrayType);

        REQUIRE_AST_MATCHES(expr, R"((ArrayType
          (QualifiedPath
            (PathSegment mod)
            (PathSegment Type))))");
    }
}

TEST_CASE("Array Type Parsing - Nested Arrays", "[parser][types][complex][array]") {
    SECTION("2D fixed-size array") {
        auto fixture = createParserFixture("[10][20]i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astArrayType);

        REQUIRE_AST_MATCHES(expr, R"((ArrayType (Int 10)
          (ArrayType (Int 20) (Type i32))))");
    }

    SECTION("Dynamic array of fixed arrays") {
        auto fixture = createParserFixture("[][10]string");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astArrayType);

        REQUIRE_AST_MATCHES(expr, R"((ArrayType
          (ArrayType (Int 10) (Type string))))");
    }

    SECTION("Fixed array of dynamic arrays") {
        auto fixture = createParserFixture("[5][]f64");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astArrayType);

        REQUIRE_AST_MATCHES(expr, R"((ArrayType (Int 5)
          (ArrayType (Type f64))))");
    }

    SECTION("3D array") {
        auto fixture = createParserFixture("[2][3][4]bool");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astArrayType);

        REQUIRE_AST_MATCHES(expr, R"((ArrayType (Int 2)
          (ArrayType (Int 3)
            (ArrayType (Int 4) (Type bool)))))");
    }
}

TEST_CASE("Array Type Parsing - Error Cases", "[parser][types][complex][array][errors]") {
    SECTION("Missing closing bracket") {
        auto fixture = createParserFixture("[10i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing element type") {
        auto fixture = createParserFixture("[10]");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Invalid size expression") {
        auto fixture = createParserFixture("[++]i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Empty array size expression") {
        auto fixture = createParserFixture("i32");
        auto *expr = fixture->parseTypeExpression();

        // Should parse as primitive type, not array
        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astPrimitiveType);
    }
}

TEST_CASE("Array Type Parsing - Integration with Declarations", "[parser][types][complex][array][integration]") {
    SECTION("Variable declaration with array type") {
        auto fixture = createParserFixture("var data: [100]i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
          (Identifier data)
          (ArrayType (Int 100) (Type i32))))");
    }

    SECTION("Function parameter with dynamic array") {
        auto fixture = createParserFixture("func process(items []string)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
          (Identifier process)
          (FuncParamDeclaration
            (Identifier items)
            (ArrayType (Type string)))))");
    }

    SECTION("Function return type with 2D array") {
        auto fixture = createParserFixture("func create() [10][20]f64");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
          (Identifier create)
          (ArrayType (Int 10)
            (ArrayType (Int 20) (Type f64)))))");
    }
}

TEST_CASE("Tuple Type Parsing - Basic Tuples", "[parser][types][complex][tuple]") {
    SECTION("Simple two-element tuple") {
        auto fixture = createParserFixture("(i32, string)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astTupleType);

        REQUIRE_AST_MATCHES(expr, R"((TupleType (Type i32) (Type string)))");
    }

    SECTION("Three-element tuple") {
        auto fixture = createParserFixture("(i32, string, bool)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astTupleType);

        REQUIRE_AST_MATCHES(expr, R"((TupleType (Type i32) (Type string) (Type bool)))");
    }

    SECTION("Mixed primitive and qualified types") {
        auto fixture = createParserFixture("(i32, mod.Type)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astTupleType);

        REQUIRE_AST_MATCHES(expr, R"((TupleType 
          (Type i32) 
          (QualifiedPath
            (PathSegment mod)
            (PathSegment Type))))");
    }

    SECTION("Empty tuple (unit type)") {
        auto fixture = createParserFixture("()");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astTupleType);

        REQUIRE_AST_MATCHES(expr, R"((TupleType))");
    }
}

TEST_CASE("Tuple Type Parsing - Nested Tuples", "[parser][types][complex][tuple]") {
    SECTION("Tuple containing another tuple") {
        auto fixture = createParserFixture("(i32, (string, bool))");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astTupleType);

        REQUIRE_AST_MATCHES(expr, R"((TupleType 
          (Type i32) 
          (TupleType (Type string) (Type bool))))");
    }

    SECTION("Tuple with array types") {
        auto fixture = createParserFixture("([10]i32, []string)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astTupleType);

        REQUIRE_AST_MATCHES(expr, R"((TupleType 
          (ArrayType (Int 10) (Type i32))
          (ArrayType (Type string))))");
    }

    SECTION("Complex nested structure") {
        auto fixture = createParserFixture("((i32, string), ([5]bool, f64))");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astTupleType);

        REQUIRE_AST_MATCHES(expr, R"((TupleType 
          (TupleType (Type i32) (Type string))
          (TupleType 
            (ArrayType (Int 5) (Type bool))
            (Type f64))))");
    }
}

TEST_CASE("Tuple Type Parsing - Error Cases", "[parser][types][complex][tuple][errors]") {
    SECTION("Missing closing parenthesis") {
        auto fixture = createParserFixture("(i32, string");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Trailing comma without element") {
        auto fixture = createParserFixture("(i32, string,)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing comma between elements") {
        auto fixture = createParserFixture("(i32 string)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Empty element") {
        auto fixture = createParserFixture("(i32, , string)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Tuple Type Parsing - Integration with Declarations", "[parser][types][complex][tuple][integration]") {
    SECTION("Variable declaration with tuple type") {
        auto fixture = createParserFixture("var point: (f64, f64)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
          (Identifier point)
          (TupleType (Type f64) (Type f64))))");
    }

    SECTION("Function parameter with tuple") {
        auto fixture = createParserFixture("func process(data (string, i32, bool))");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
          (Identifier process)
          (FuncParamDeclaration
            (Identifier data)
            (TupleType (Type string) (Type i32) (Type bool)))))");
    }

    SECTION("Function return type with tuple") {
        auto fixture = createParserFixture("func divide(a f64, b f64) (f64, bool)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
          (Identifier divide)
          (FuncParamDeclaration
            (Identifier a)
            (Type f64))
          (FuncParamDeclaration
            (Identifier b)
            (Type f64))
          (TupleType (Type f64) (Type bool))))");
    }
}

TEST_CASE("Union Type Parsing - Basic Unions", "[parser][types][complex][union]") {
    SECTION("Simple two-type union") {
        auto fixture = createParserFixture("i32|string");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astUnionType);

        REQUIRE_AST_MATCHES(expr, R"((UnionType (Type i32) (Type string)))");
    }

    SECTION("Three-type union") {
        auto fixture = createParserFixture("i32|string|bool");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astUnionType);

        REQUIRE_AST_MATCHES(expr, R"((UnionType (Type i32) (Type string) (Type bool)))");
    }

    SECTION("Union with qualified types") {
        auto fixture = createParserFixture("i32|mod.Type|Error");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astUnionType);

        REQUIRE_AST_MATCHES(expr, R"((UnionType 
          (Type i32) 
          (QualifiedPath
            (PathSegment mod)
            (PathSegment Type))
          (Identifier Error)))");
    }

    SECTION("Union with error type") {
        auto fixture = createParserFixture("User|Error");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astUnionType);

        REQUIRE_AST_MATCHES(expr, R"((UnionType (Identifier User) (Identifier Error)))");
    }
}

TEST_CASE("Union Type Parsing - Complex Union Types", "[parser][types][complex][union]") {
    SECTION("Union with array types") {
        auto fixture = createParserFixture("[10]i32|[]string");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astUnionType);

        REQUIRE_AST_MATCHES(expr, R"((UnionType 
          (ArrayType (Int 10) (Type i32))
          (ArrayType (Type string))))");
    }

    SECTION("Union with tuple types") {
        auto fixture = createParserFixture("(i32, string)|(bool, f64)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astUnionType);

        REQUIRE_AST_MATCHES(expr, R"((UnionType 
          (TupleType (Type i32) (Type string))
          (TupleType (Type bool) (Type f64))))");
    }

    SECTION("Nested unions with precedence") {
        auto fixture = createParserFixture("i32|string|Error|Result");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astUnionType);

        REQUIRE_AST_MATCHES(expr, R"((UnionType 
          (Type i32) 
          (Type string) 
          (Identifier Error) 
          (Identifier Result)))");
    }

    SECTION("Union with complex nested types") {
        auto fixture = createParserFixture("([5]i32, string)|([]bool, f64)|Error");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astUnionType);

        REQUIRE_AST_MATCHES(expr, R"((UnionType 
          (TupleType 
            (ArrayType (Int 5) (Type i32)) 
            (Type string))
          (TupleType 
            (ArrayType (Type bool)) 
            (Type f64))
          (Identifier Error)))");
    }
}

TEST_CASE("Union Type Parsing - Error Cases", "[parser][types][complex][union][errors]") {
    SECTION("Single type with | should fail") {
        auto fixture = createParserFixture("i32|");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Leading | should fail") {
        auto fixture = createParserFixture("|i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Double | parses only first type") {
        auto fixture = createParserFixture("i32||string");
        auto *expr = fixture->parseTypeExpression();

        // Should parse i32 successfully and stop at ||
        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astPrimitiveType);
        REQUIRE(!fixture->hasErrors());
    }

    SECTION("Empty union member should fail") {
        auto fixture = createParserFixture("i32| |string");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Union Type Parsing - Integration with Declarations", "[parser][types][complex][union][integration]") {
    SECTION("Variable declaration with union type") {
        auto fixture = createParserFixture("var result: i32|Error");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
          (Identifier result)
          (UnionType (Type i32) (Identifier Error))))");
    }

    SECTION("Function parameter with union") {
        auto fixture = createParserFixture("func process(data i32|string|bool)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
          (Identifier process)
          (FuncParamDeclaration
            (Identifier data)
            (UnionType (Type i32) (Type string) (Type bool)))))");
    }

    SECTION("Function return type with union") {
        auto fixture = createParserFixture("func getValue() i32|Error");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
          (Identifier getValue)
          (UnionType (Type i32) (Identifier Error))))");
    }
}

TEST_CASE("Reference Type Parsing - Basic References", "[parser][types][complex][reference]") {
   SECTION("Simple reference type") {
       auto fixture = createParserFixture("&i32");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astReferenceType);

       REQUIRE_AST_MATCHES(expr, R"((ReferenceType (Type i32)))");
   }

   SECTION("Reference to string") {
       auto fixture = createParserFixture("&string");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astReferenceType);

       REQUIRE_AST_MATCHES(expr, R"((ReferenceType (Type string)))");
   }

   SECTION("Reference to qualified type") {
       auto fixture = createParserFixture("&mod.Type");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astReferenceType);

       REQUIRE_AST_MATCHES(expr, R"((ReferenceType 
         (QualifiedPath
           (PathSegment mod)
           (PathSegment Type))))");
   }

   SECTION("Reference to identifier") {
       auto fixture = createParserFixture("&User");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astReferenceType);

       REQUIRE_AST_MATCHES(expr, R"((ReferenceType (Identifier User)))");
   }
}

TEST_CASE("Reference Type Parsing - Complex References", "[parser][types][complex][reference]") {
   SECTION("Reference to array") {
       auto fixture = createParserFixture("&[10]i32");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astReferenceType);

       REQUIRE_AST_MATCHES(expr, R"((ReferenceType 
         (ArrayType (Int 10) (Type i32))))");
   }

   SECTION("Reference to tuple") {
       auto fixture = createParserFixture("&(i32, string)");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astReferenceType);

       REQUIRE_AST_MATCHES(expr, R"((ReferenceType 
         (TupleType (Type i32) (Type string))))");
   }

   SECTION("Reference to union") {
       auto fixture = createParserFixture("&(i32|string)");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astReferenceType);

       REQUIRE_AST_MATCHES(expr, R"((ReferenceType 
         (UnionType (Type i32) (Type string))))");
   }

   SECTION("Double reference parses only first &") {
       auto fixture = createParserFixture("&&i32");
       auto *expr = fixture->parseTypeExpression();

       // Should fail because && is LAnd token, not two BAnd tokens
       REQUIRE(expr == nullptr);
       REQUIRE(fixture->hasErrors());
   }

   SECTION("Array of references") {
       auto fixture = createParserFixture("[10]&i32");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astArrayType);

       REQUIRE_AST_MATCHES(expr, R"((ArrayType (Int 10)
         (ReferenceType (Type i32))))");
   }
}

TEST_CASE("Reference Type Parsing - Error Cases", "[parser][types][complex][reference][errors]") {
   SECTION("Reference without target type") {
       auto fixture = createParserFixture("&");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr == nullptr);
       REQUIRE(fixture->hasErrors());
   }

   SECTION("Reference to invalid type") {
       auto fixture = createParserFixture("&123");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr == nullptr);
       REQUIRE(fixture->hasErrors());
   }
}

TEST_CASE("Reference Type Parsing - Integration with Declarations", "[parser][types][complex][reference][integration]") {
   SECTION("Variable declaration with reference type") {
       auto fixture = createParserFixture("var ptr: &i32");
       auto *stmt = fixture->parseDeclaration();

       REQUIRE(stmt != nullptr);
       REQUIRE(stmt->kind == astVariableDeclaration);

       REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
         (Identifier ptr)
         (ReferenceType (Type i32))))");
   }

   SECTION("Function parameter with reference") {
       auto fixture = createParserFixture("func process(data &string)");
       auto *stmt = fixture->parseDeclaration();

       REQUIRE(stmt != nullptr);
       REQUIRE(stmt->kind == astFuncDeclaration);

       REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
         (Identifier process)
         (FuncParamDeclaration
           (Identifier data)
           (ReferenceType (Type string)))))");
   }

   SECTION("Function return type with reference") {
       auto fixture = createParserFixture("func getData() &User");
       auto *stmt = fixture->parseDeclaration();

       REQUIRE(stmt != nullptr);
       REQUIRE(stmt->kind == astFuncDeclaration);

       REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
         (Identifier getData)
         (ReferenceType (Identifier User))))");
   }
}

TEST_CASE("Pointer Type Parsing - Basic Pointers", "[parser][types][complex][pointer]") {
    SECTION("Simple pointer type") {
        auto fixture = createParserFixture("*i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astPointerType);

        REQUIRE_AST_MATCHES(expr, R"((PointerType (Type i32)))");
    }

    SECTION("Pointer to string") {
        auto fixture = createParserFixture("*string");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astPointerType);

        REQUIRE_AST_MATCHES(expr, R"((PointerType (Type string)))");
    }

    SECTION("Pointer to qualified type") {
        auto fixture = createParserFixture("*mod.Type");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astPointerType);

        REQUIRE_AST_MATCHES(expr, R"((PointerType 
          (QualifiedPath
            (PathSegment mod)
            (PathSegment Type))))");
    }

    SECTION("Pointer to identifier") {
        auto fixture = createParserFixture("*User");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astPointerType);

        REQUIRE_AST_MATCHES(expr, R"((PointerType (Identifier User)))");
    }
}

TEST_CASE("Pointer Type Parsing - Complex Pointers", "[parser][types][complex][pointer]") {
    SECTION("Pointer to array") {
        auto fixture = createParserFixture("*[10]i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astPointerType);

        REQUIRE_AST_MATCHES(expr, R"((PointerType 
          (ArrayType (Int 10) (Type i32))))");
    }

    SECTION("Pointer to tuple") {
        auto fixture = createParserFixture("*(i32, string)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astPointerType);

        REQUIRE_AST_MATCHES(expr, R"((PointerType 
          (TupleType (Type i32) (Type string))))");
    }

    SECTION("Pointer to union") {
        auto fixture = createParserFixture("*(i32|string)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astPointerType);

        REQUIRE_AST_MATCHES(expr, R"((PointerType 
          (UnionType (Type i32) (Type string))))");
    }

    SECTION("Double pointer") {
        auto fixture = createParserFixture("**i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astPointerType);

        REQUIRE_AST_MATCHES(expr, R"((PointerType 
          (PointerType (Type i32))))");
    }

    SECTION("Array of pointers") {
        auto fixture = createParserFixture("[10]*i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astArrayType);

        REQUIRE_AST_MATCHES(expr, R"((ArrayType (Int 10)
          (PointerType (Type i32))))");
    }

    SECTION("Pointer and reference combination") {
        auto fixture = createParserFixture("*&i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astPointerType);

        REQUIRE_AST_MATCHES(expr, R"((PointerType 
          (ReferenceType (Type i32))))");
    }

    SECTION("Reference to pointer") {
        auto fixture = createParserFixture("&*i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astReferenceType);

        REQUIRE_AST_MATCHES(expr, R"((ReferenceType 
          (PointerType (Type i32))))");
    }
}

TEST_CASE("Pointer Type Parsing - Error Cases", "[parser][types][complex][pointer][errors]") {
    SECTION("Pointer without target type") {
        auto fixture = createParserFixture("*");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Pointer to invalid type") {
        auto fixture = createParserFixture("*123");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Pointer Type Parsing - Integration with Declarations", "[parser][types][complex][pointer][integration]") {
    SECTION("Variable declaration with pointer type") {
        auto fixture = createParserFixture("var ptr: *i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
          (Identifier ptr)
          (PointerType (Type i32))))");
    }

    SECTION("Function parameter with pointer") {
        auto fixture = createParserFixture("func process(data *string)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
          (Identifier process)
          (FuncParamDeclaration
            (Identifier data)
            (PointerType (Type string)))))");
    }

    SECTION("Function return type with pointer") {
        auto fixture = createParserFixture("func getData() *User");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
          (Identifier getData)
          (PointerType (Identifier User))))");
    }
}

TEST_CASE("Optional Type Parsing - Basic Optionals", "[parser][types][complex][optional]") {
    SECTION("Simple optional type") {
        auto fixture = createParserFixture("?i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astOptionalType);

        REQUIRE_AST_MATCHES(expr, R"((OptionalType (Type i32)))");
    }

    SECTION("Optional string") {
        auto fixture = createParserFixture("?string");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astOptionalType);

        REQUIRE_AST_MATCHES(expr, R"((OptionalType (Type string)))");
    }

    SECTION("Optional qualified type") {
        auto fixture = createParserFixture("?mod.Type");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astOptionalType);

        REQUIRE_AST_MATCHES(expr, R"((OptionalType 
          (QualifiedPath
            (PathSegment mod)
            (PathSegment Type))))");
    }

    SECTION("Optional identifier") {
        auto fixture = createParserFixture("?User");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astOptionalType);

        REQUIRE_AST_MATCHES(expr, R"((OptionalType (Identifier User)))");
    }
}

TEST_CASE("Optional Type Parsing - Complex Optionals", "[parser][types][complex][optional]") {
    SECTION("Optional array") {
        auto fixture = createParserFixture("?[10]i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astOptionalType);

        REQUIRE_AST_MATCHES(expr, R"((OptionalType 
          (ArrayType (Int 10) (Type i32))))");
    }

    SECTION("Optional tuple") {
        auto fixture = createParserFixture("?(i32, string)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astOptionalType);

        REQUIRE_AST_MATCHES(expr, R"((OptionalType 
          (TupleType (Type i32) (Type string))))");
    }

    SECTION("Optional union") {
        auto fixture = createParserFixture("?(i32|string)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astOptionalType);

        REQUIRE_AST_MATCHES(expr, R"((OptionalType 
          (UnionType (Type i32) (Type string))))");
    }

    SECTION("Optional pointer") {
        auto fixture = createParserFixture("?*i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astOptionalType);

        REQUIRE_AST_MATCHES(expr, R"((OptionalType 
          (PointerType (Type i32))))");
    }

    SECTION("Optional reference") {
        auto fixture = createParserFixture("?&i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astOptionalType);

        REQUIRE_AST_MATCHES(expr, R"((OptionalType 
          (ReferenceType (Type i32))))");
    }

    SECTION("Array of optionals") {
        auto fixture = createParserFixture("[10]?i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astArrayType);

        REQUIRE_AST_MATCHES(expr, R"((ArrayType (Int 10)
          (OptionalType (Type i32))))");
    }

    SECTION("Optional of optional (double optional)") {
        auto fixture = createParserFixture("??i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astOptionalType);

        REQUIRE_AST_MATCHES(expr, R"((OptionalType 
          (OptionalType (Type i32))))");
    }
}

TEST_CASE("Optional Type Parsing - Error Cases", "[parser][types][complex][optional][errors]") {
    SECTION("Optional without target type") {
        auto fixture = createParserFixture("?");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Optional with invalid type") {
        auto fixture = createParserFixture("?123");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Optional Type Parsing - Integration with Declarations", "[parser][types][complex][optional][integration]") {
    SECTION("Variable declaration with optional type") {
        auto fixture = createParserFixture("var value: ?i32");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
          (Identifier value)
          (OptionalType (Type i32))))");
    }

    SECTION("Function parameter with optional") {
        auto fixture = createParserFixture("func process(data ?string)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
          (Identifier process)
          (FuncParamDeclaration
            (Identifier data)
            (OptionalType (Type string)))))");
    }

    SECTION("Function return type with optional") {
        auto fixture = createParserFixture("func findUser() ?User");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
          (Identifier findUser)
          (OptionalType (Identifier User))))");
    }
}

TEST_CASE("Result Type Parsing - Basic Results", "[parser][types][complex][result]") {
   SECTION("Simple result type") {
       auto fixture = createParserFixture("!i32");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astResultType);

       REQUIRE_AST_MATCHES(expr, R"((ResultType (Type i32)))");
   }

   SECTION("Result string") {
       auto fixture = createParserFixture("!string");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astResultType);

       REQUIRE_AST_MATCHES(expr, R"((ResultType (Type string)))");
   }

   SECTION("Result qualified type") {
       auto fixture = createParserFixture("!mod.Type");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astResultType);

       REQUIRE_AST_MATCHES(expr, R"((ResultType 
         (QualifiedPath
           (PathSegment mod)
           (PathSegment Type))))");
   }

   SECTION("Result identifier") {
       auto fixture = createParserFixture("!User");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astResultType);

       REQUIRE_AST_MATCHES(expr, R"((ResultType (Identifier User)))");
   }
}

TEST_CASE("Result Type Parsing - Complex Results", "[parser][types][complex][result]") {
   SECTION("Result array") {
       auto fixture = createParserFixture("![10]i32");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astResultType);

       REQUIRE_AST_MATCHES(expr, R"((ResultType 
         (ArrayType (Int 10) (Type i32))))");
   }

   SECTION("Result tuple") {
       auto fixture = createParserFixture("!(i32, string)");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astResultType);

       REQUIRE_AST_MATCHES(expr, R"((ResultType 
         (TupleType (Type i32) (Type string))))");
   }

   SECTION("Result union") {
       auto fixture = createParserFixture("!(i32|string)");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astResultType);

       REQUIRE_AST_MATCHES(expr, R"((ResultType 
         (UnionType (Type i32) (Type string))))");
   }

   SECTION("Result pointer") {
       auto fixture = createParserFixture("!*i32");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astResultType);

       REQUIRE_AST_MATCHES(expr, R"((ResultType 
         (PointerType (Type i32))))");
   }

   SECTION("Result reference") {
       auto fixture = createParserFixture("!&i32");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astResultType);

       REQUIRE_AST_MATCHES(expr, R"((ResultType 
         (ReferenceType (Type i32))))");
   }

   SECTION("Result optional") {
       auto fixture = createParserFixture("!?i32");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astResultType);

       REQUIRE_AST_MATCHES(expr, R"((ResultType 
         (OptionalType (Type i32))))");
   }

   SECTION("Array of results") {
       auto fixture = createParserFixture("[10]!i32");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astArrayType);

       REQUIRE_AST_MATCHES(expr, R"((ArrayType (Int 10)
         (ResultType (Type i32))))");
   }

   SECTION("Result of result (double result)") {
       auto fixture = createParserFixture("!!i32");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr != nullptr);
       REQUIRE(expr->kind == astResultType);

       REQUIRE_AST_MATCHES(expr, R"((ResultType 
         (ResultType (Type i32))))");
   }
}

TEST_CASE("Result Type Parsing - Error Cases", "[parser][types][complex][result][errors]") {
   SECTION("Result without target type") {
       auto fixture = createParserFixture("!");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr == nullptr);
       REQUIRE(fixture->hasErrors());
   }

   SECTION("Result with invalid type") {
       auto fixture = createParserFixture("!123");
       auto *expr = fixture->parseTypeExpression();

       REQUIRE(expr == nullptr);
       REQUIRE(fixture->hasErrors());
   }
}

TEST_CASE("Result Type Parsing - Integration with Declarations", "[parser][types][complex][result][integration]") {
   SECTION("Variable declaration with result type") {
       auto fixture = createParserFixture("var value: !i32");
       auto *stmt = fixture->parseDeclaration();

       REQUIRE(stmt != nullptr);
       REQUIRE(stmt->kind == astVariableDeclaration);

       REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
         (Identifier value)
         (ResultType (Type i32))))");
   }

   SECTION("Function parameter with result") {
       auto fixture = createParserFixture("func process(data !string)");
       auto *stmt = fixture->parseDeclaration();

       REQUIRE(stmt != nullptr);
       REQUIRE(stmt->kind == astFuncDeclaration);

       REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
         (Identifier process)
         (FuncParamDeclaration
           (Identifier data)
           (ResultType (Type string)))))");
   }

   SECTION("Function return type with result") {
       auto fixture = createParserFixture("func tryOperation() !User");
       auto *stmt = fixture->parseDeclaration();

       REQUIRE(stmt != nullptr);
       REQUIRE(stmt->kind == astFuncDeclaration);

       REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
         (Identifier tryOperation)
         (ResultType (Identifier User))))");
   }
}

TEST_CASE("Function Type Parsing - Basic Functions", "[parser][types][complex][function]") {
    SECTION("Simple function type") {
        auto fixture = createParserFixture("func(i32) -> string");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astFunctionType);

        REQUIRE_AST_MATCHES(expr, R"((FunctionType (Type i32) (Type string)))");
    }

    SECTION("Function with no parameters") {
        auto fixture = createParserFixture("func() -> i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astFunctionType);

        REQUIRE_AST_MATCHES(expr, R"((FunctionType (Type i32)))");
    }

    SECTION("Function with multiple parameters") {
        auto fixture = createParserFixture("func(i32, string, bool) -> f64");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astFunctionType);

        REQUIRE_AST_MATCHES(expr, R"((FunctionType (Type i32) (Type string) (Type bool) (Type f64)))");
    }

    SECTION("Function with qualified parameter types") {
        auto fixture = createParserFixture("func(mod.Type, &string) -> *User");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astFunctionType);

        REQUIRE_AST_MATCHES(expr, R"((FunctionType 
          (QualifiedPath
            (PathSegment mod)
            (PathSegment Type))
          (ReferenceType (Type string))
          (PointerType (Identifier User))))");
    }
}

TEST_CASE("Function Type Parsing - Complex Functions", "[parser][types][complex][function]") {
    SECTION("Function with array parameters") {
        auto fixture = createParserFixture("func([10]i32, []string) -> bool");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astFunctionType);

        REQUIRE_AST_MATCHES(expr, R"((FunctionType 
          (ArrayType (Int 10) (Type i32))
          (ArrayType (Type string))
          (Type bool)))");
    }

    SECTION("Function with tuple parameters") {
        auto fixture = createParserFixture("func((i32, string), bool) -> (f64, bool)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astFunctionType);

        REQUIRE_AST_MATCHES(expr, R"((FunctionType 
          (TupleType (Type i32) (Type string))
          (Type bool)
          (TupleType (Type f64) (Type bool))))");
    }

    SECTION("Function with union parameters") {
        auto fixture = createParserFixture("func((i32|string), bool) -> ?User");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astFunctionType);

        REQUIRE_AST_MATCHES(expr, R"((FunctionType 
          (UnionType (Type i32) (Type string))
          (Type bool)
          (OptionalType (Identifier User))))");
    }

    SECTION("Function with optional and result parameters") {
        auto fixture = createParserFixture("func(?i32, !string) -> !?User");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astFunctionType);

        REQUIRE_AST_MATCHES(expr, R"((FunctionType 
          (OptionalType (Type i32))
          (ResultType (Type string))
          (ResultType (OptionalType (Identifier User)))))");
    }

    SECTION("Higher-order function") {
        auto fixture = createParserFixture("func(func(i32) -> bool, string) -> []i32");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astFunctionType);

        REQUIRE_AST_MATCHES(expr, R"((FunctionType 
          (FunctionType (Type i32) (Type bool))
          (Type string)
          (ArrayType (Type i32))))");
    }

    SECTION("Function returning function") {
        auto fixture = createParserFixture("func(i32) -> func(string) -> bool");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr != nullptr);
        REQUIRE(expr->kind == astFunctionType);

        REQUIRE_AST_MATCHES(expr, R"((FunctionType 
          (Type i32)
          (FunctionType (Type string) (Type bool))))");
    }
}

TEST_CASE("Function Type Parsing - Error Cases", "[parser][types][complex][function][errors]") {
    SECTION("Function without return type") {
        auto fixture = createParserFixture("func(i32)");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Function with missing arrow") {
        auto fixture = createParserFixture("func(i32) string");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Function with missing closing paren") {
        auto fixture = createParserFixture("func(i32, string -> bool");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Function with trailing comma") {
        auto fixture = createParserFixture("func(i32, string,) -> bool");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Function with empty parameter") {
        auto fixture = createParserFixture("func(i32, , string) -> bool");
        auto *expr = fixture->parseTypeExpression();

        REQUIRE(expr == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Function Type Parsing - Integration with Declarations", "[parser][types][complex][function][integration]") {
    SECTION("Variable declaration with function type") {
        auto fixture = createParserFixture("var callback: func(i32) -> bool");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
          (Identifier callback)
          (FunctionType (Type i32) (Type bool))))");
    }

    SECTION("Function parameter with function type") {
        auto fixture = createParserFixture("func process(callback func(string) -> i32)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
          (Identifier process)
          (FuncParamDeclaration
            (Identifier callback)
            (FunctionType (Type string) (Type i32)))))");
    }

    SECTION("Function return type with function type") {
        auto fixture = createParserFixture("func createHandler() func(i32, string) -> bool");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astFuncDeclaration);

        REQUIRE_AST_MATCHES(stmt, R"((FuncDeclaration
          (Identifier createHandler)
          (FunctionType (Type i32) (Type string) (Type bool))))");
    }
}
