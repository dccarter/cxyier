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

TEST_CASE("Enum Declaration Parsing - Basic Forms", "[parser][declarations][enum-decl]") {
    SECTION("enum Color { Red }") {
        auto fixture = createParserFixture("enum Color { Red }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->name != nullptr);
        REQUIRE(enumDecl->base == nullptr);
        REQUIRE(enumDecl->options.size() == 1);

        auto *option = static_cast<EnumOptionDeclarationNode *>(enumDecl->options[0]);
        REQUIRE(option->name != nullptr);
        REQUIRE(option->value == nullptr);

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier Color)
  (EnumOptionDeclaration
    (Identifier Red))))");
    }

    SECTION("enum Color { Red, Green, Blue }") {
        auto fixture = createParserFixture("enum Color { Red, Green, Blue }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->name != nullptr);
        REQUIRE(enumDecl->base == nullptr);
        REQUIRE(enumDecl->options.size() == 3);

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier Color)
  (EnumOptionDeclaration
    (Identifier Red))
  (EnumOptionDeclaration
    (Identifier Green))
  (EnumOptionDeclaration
    (Identifier Blue))))");
    }

    SECTION("enum Empty {}") {
        auto fixture = createParserFixture("enum Empty {}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->name != nullptr);
        REQUIRE(enumDecl->base == nullptr);
        REQUIRE(enumDecl->options.empty());

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier Empty)))");
    }
}

TEST_CASE("Enum Declaration Parsing - Explicit Values", "[parser][declarations][enum-decl][values]") {
    SECTION("enum StatusCode { Ok = 200 }") {
        auto fixture = createParserFixture("enum StatusCode { Ok = 200 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->options.size() == 1);

        auto *option = static_cast<EnumOptionDeclarationNode *>(enumDecl->options[0]);
        REQUIRE(option->value != nullptr);
        REQUIRE(option->value->kind == astInt);

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier StatusCode)
  (EnumOptionDeclaration
    (Identifier Ok)
    (Int 200))))");
    }

    SECTION("enum StatusCode { Ok = 200, NotFound = 404, InternalError = 500 }") {
        auto fixture = createParserFixture("enum StatusCode { Ok = 200, NotFound = 404, InternalError = 500 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->options.size() == 3);

        // Check all options have values
        for (auto *opt : enumDecl->options) {
            auto *option = static_cast<EnumOptionDeclarationNode *>(opt);
            REQUIRE(option->value != nullptr);
            REQUIRE(option->value->kind == astInt);
        }

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier StatusCode)
  (EnumOptionDeclaration
    (Identifier Ok)
    (Int 200))
  (EnumOptionDeclaration
    (Identifier NotFound)
    (Int 404))
  (EnumOptionDeclaration
    (Identifier InternalError)
    (Int 500))))");
    }

    SECTION("enum Mixed { First, Second = 10, Third }") {
        auto fixture = createParserFixture("enum Mixed { First, Second = 10, Third }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->options.size() == 3);

        auto *first = static_cast<EnumOptionDeclarationNode *>(enumDecl->options[0]);
        auto *second = static_cast<EnumOptionDeclarationNode *>(enumDecl->options[1]);
        auto *third = static_cast<EnumOptionDeclarationNode *>(enumDecl->options[2]);

        REQUIRE(first->value == nullptr);   // auto value
        REQUIRE(second->value != nullptr);  // explicit value
        REQUIRE(third->value == nullptr);   // auto value

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier Mixed)
  (EnumOptionDeclaration
    (Identifier First))
  (EnumOptionDeclaration
    (Identifier Second)
    (Int 10))
  (EnumOptionDeclaration
    (Identifier Third))))");
    }
}

TEST_CASE("Enum Declaration Parsing - Backing Types", "[parser][declarations][enum-decl][backing-type]") {
    SECTION("enum Priority : i8 { Low, Medium, High }") {
        auto fixture = createParserFixture("enum Priority : i8 { Low, Medium, High }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->name != nullptr);
        REQUIRE(enumDecl->base != nullptr);
        REQUIRE(enumDecl->base->kind == astPrimitiveType);
        REQUIRE(enumDecl->options.size() == 3);

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier Priority)
  (Type i8)
  (EnumOptionDeclaration
    (Identifier Low))
  (EnumOptionDeclaration
    (Identifier Medium))
  (EnumOptionDeclaration
    (Identifier High))))");
    }

    SECTION("enum Flags : u32 { Read = 1, Write = 2, Execute = 4 }") {
        auto fixture = createParserFixture("enum Flags : u32 { Read = 1, Write = 2, Execute = 4 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->base != nullptr);
        REQUIRE(enumDecl->base->kind == astPrimitiveType);
        REQUIRE(enumDecl->options.size() == 3);

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier Flags)
  (Type u32)
  (EnumOptionDeclaration
    (Identifier Read)
    (Int 1))
  (EnumOptionDeclaration
    (Identifier Write)
    (Int 2))
  (EnumOptionDeclaration
    (Identifier Execute)
    (Int 4))))");
    }

    SECTION("enum Flags : i8 {}") {
        auto fixture = createParserFixture("enum Flags : i8 {}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->base != nullptr);
        REQUIRE(enumDecl->base->kind == astPrimitiveType);
        REQUIRE(enumDecl->options.empty());

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier Flags)
  (Type i8)))");
    }
}

TEST_CASE("Enum Declaration Parsing - With Attributes", "[parser][declarations][enum-decl][attributes]") {
    SECTION("@repr(\"C\") enum TokenKind { Eof }") {
        auto fixture = createParserFixture("@repr(\"C\") enum TokenKind { Eof }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);
        REQUIRE(stmt->hasAttributes());
        REQUIRE(stmt->getAttributeCount() == 1);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->options.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier TokenKind)
  (EnumOptionDeclaration
    (Identifier Eof))))");
    }

    SECTION("enum Hello { @str(\"one\") One, Two, Three = 3 }") {
        auto fixture = createParserFixture("enum Hello { @str(\"one\") One, Two, Three = 3 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->options.size() == 3);

        // Check that first option has attributes
        auto *firstOption = static_cast<EnumOptionDeclarationNode *>(enumDecl->options[0]);
        REQUIRE(firstOption->hasAttributes());
        REQUIRE(firstOption->getAttributeCount() == 1);

        // Check that third option has explicit value
        auto *thirdOption = static_cast<EnumOptionDeclarationNode *>(enumDecl->options[2]);
        REQUIRE(thirdOption->value != nullptr);
        REQUIRE(thirdOption->value->kind == astInt);

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier Hello)
  (EnumOptionDeclaration
    (Identifier One))
  (EnumOptionDeclaration
    (Identifier Two))
  (EnumOptionDeclaration
    (Identifier Three)
    (Int 3))))");
    }

    SECTION("enum HttpMethod { @str(\"GET\") Get, @str(\"POST\") Post, @deprecated Put }") {
        auto fixture = createParserFixture("enum HttpMethod { @str(\"GET\") Get, @str(\"POST\") Post, @deprecated Put }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->options.size() == 3);

        // All options should have attributes
        for (auto *opt : enumDecl->options) {
            auto *option = static_cast<EnumOptionDeclarationNode *>(opt);
            REQUIRE(option->hasAttributes());
            REQUIRE(option->getAttributeCount() == 1);
        }

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier HttpMethod)
  (EnumOptionDeclaration
    (Identifier Get))
  (EnumOptionDeclaration
    (Identifier Post))
  (EnumOptionDeclaration
    (Identifier Put))))");
    }
}

TEST_CASE("Enum Declaration Parsing - Trailing Commas", "[parser][declarations][enum-decl][trailing-comma]") {
    SECTION("enum Color { Red, Green, Blue, }") {
        auto fixture = createParserFixture("enum Color { Red, Green, Blue, }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->options.size() == 3);

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier Color)
  (EnumOptionDeclaration
    (Identifier Red))
  (EnumOptionDeclaration
    (Identifier Green))
  (EnumOptionDeclaration
    (Identifier Blue))))");
    }

    SECTION("enum StatusCode { Ok = 200, }") {
        auto fixture = createParserFixture("enum StatusCode { Ok = 200, }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->options.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier StatusCode)
  (EnumOptionDeclaration
    (Identifier Ok)
    (Int 200))))");
    }
}

TEST_CASE("Enum Declaration Parsing - Error Cases", "[parser][declarations][enum-decl][errors]") {
    SECTION("enum without name") {
        auto fixture = createParserFixture("enum");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("enum without body") {
        auto fixture = createParserFixture("enum Color");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("enum with unclosed body") {
        auto fixture = createParserFixture("enum Color { Red");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("enum with invalid backing type") {
        auto fixture = createParserFixture("enum Color : { Red }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("enum option without name") {
        auto fixture = createParserFixture("enum Color { = 1 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("enum option with invalid value expression") {
        auto fixture = createParserFixture("enum Color { Red = }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("enum with missing comma between options") {
        auto fixture = createParserFixture("enum Color { Red Green }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Enum Declaration Parsing - Complex Cases", "[parser][declarations][enum-decl][complex]") {
    SECTION("enum with all features") {
        auto fixture = createParserFixture(R"(
@repr("C")
enum TokenKind : u16 {
    @doc("End of file")
    Eof = 0,
    
    Comment = 10,
    
    @deprecated
    LegacyToken = 999
}
)");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);
        REQUIRE(stmt->hasAttributes());

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->base != nullptr);
        REQUIRE(enumDecl->base->kind == astPrimitiveType);
        REQUIRE(enumDecl->options.size() == 3);

        // Check first option has attributes and value
        auto *firstOption = static_cast<EnumOptionDeclarationNode *>(enumDecl->options[0]);
        REQUIRE(firstOption->hasAttributes());
        REQUIRE(firstOption->value != nullptr);

        // Check second option has value but no attributes
        auto *secondOption = static_cast<EnumOptionDeclarationNode *>(enumDecl->options[1]);
        REQUIRE(!secondOption->hasAttributes());
        REQUIRE(secondOption->value != nullptr);

        // Check third option has attributes and value
        auto *thirdOption = static_cast<EnumOptionDeclarationNode *>(enumDecl->options[2]);
        REQUIRE(thirdOption->hasAttributes());
        REQUIRE(thirdOption->value != nullptr);
    }

    SECTION("enum with expression values") {
        auto fixture = createParserFixture("enum Powers { Two = 1 + 1, Four = 2 * 2, Eight = 4 + 4 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->options.size() == 3);

        // All options should have binary expression values
        for (auto *opt : enumDecl->options) {
            auto *option = static_cast<EnumOptionDeclarationNode *>(opt);
            REQUIRE(option->value != nullptr);
            REQUIRE(option->value->kind == astBinaryExpr);
        }

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier Powers)
  (EnumOptionDeclaration
    (Identifier Two)
    (BinaryExpr + (Int 1) (Int 1)))
  (EnumOptionDeclaration
    (Identifier Four)
    (BinaryExpr * (Int 2) (Int 2)))
  (EnumOptionDeclaration
    (Identifier Eight)
    (BinaryExpr + (Int 4) (Int 4)))))");
    }
}

TEST_CASE("Enum Declaration Parsing - Visibility Modifiers", "[parser][declarations][enum-decl][visibility]") {
    SECTION("pub enum Status { Ok }") {
        auto fixture = createParserFixture("pub enum Status { Ok }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);
        REQUIRE((stmt->flags & flgPublic) != 0);
        REQUIRE((stmt->flags & flgExtern) == 0);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->name != nullptr);
        REQUIRE(enumDecl->options.size() == 1);

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier Status)
  (EnumOptionDeclaration
    (Identifier Ok))))");
    }

    SECTION("extern enum ErrorCode { NotFound } - should fail") {
        auto fixture = createParserFixture("extern enum ErrorCode { NotFound }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("pub enum Color : i8 { Red = 1, Green = 2, Blue = 3 }") {
        auto fixture = createParserFixture("pub enum Color : i8 { Red = 1, Green = 2, Blue = 3 }");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);
        REQUIRE((stmt->flags & flgPublic) != 0);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->base != nullptr);
        REQUIRE(enumDecl->base->kind == astPrimitiveType);
        REQUIRE(enumDecl->options.size() == 3);

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier Color)
  (Type i8)
  (EnumOptionDeclaration
    (Identifier Red)
    (Int 1))
  (EnumOptionDeclaration
    (Identifier Green)
    (Int 2))
  (EnumOptionDeclaration
    (Identifier Blue)
    (Int 3))))");
    }

    SECTION("@deprecated pub enum LegacyStatus {}") {
        auto fixture = createParserFixture("@deprecated pub enum LegacyStatus {}");
        auto *stmt = fixture->parseDeclaration();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astEnumDeclaration);
        REQUIRE((stmt->flags & flgPublic) != 0);
        REQUIRE(stmt->hasAttributes());
        REQUIRE(stmt->getAttributeCount() == 1);

        auto *enumDecl = static_cast<EnumDeclarationNode *>(stmt);
        REQUIRE(enumDecl->options.empty());

        REQUIRE_AST_MATCHES(stmt, R"((EnumDeclaration
  (Identifier LegacyStatus)))");
    }
}