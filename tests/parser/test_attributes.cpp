#include "catch2.hpp"
#include "../parser_test_utils.hpp"
#include "cxy/ast/declarations.hpp"
#include "cxy/ast/attributes.hpp"
#include "cxy/ast/literals.hpp"
#include "cxy/ast/identifiers.hpp"

using namespace cxy;
using namespace cxy::ast;
using namespace cxy::test;

TEST_CASE("Attribute Parsing - Simple Attributes on Variable Declarations", "[parser][attributes][declarations]") {
    SECTION("@deprecated var x = 42") {
        auto fixture = createParserFixture("@deprecated var x = 42");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 1);

        auto *attr = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr->name.view() == "deprecated");
        REQUIRE(attr->getArgCount() == 0);

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [deprecated]
  (Identifier x)
  (Int 42)))", PrinterFlags::IncludeAttributes);
    }

    SECTION("@readonly const PI = 3.14") {
        auto fixture = createParserFixture("@readonly const PI = 3.14");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 1);

        auto *attr = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr->name.view() == "readonly");
        REQUIRE(attr->getArgCount() == 0);

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [readonly]
  (Identifier PI)
  (Float 3.14)))", PrinterFlags::IncludeAttributes);
    }

    SECTION("@volatile auto value = getValue()") {
        auto fixture = createParserFixture("@volatile auto value = getValue()");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 1);

        auto *attr = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr->name.view() == "volatile");

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [volatile]
  (Identifier value)
  (CallExpr (Identifier getValue))))", PrinterFlags::IncludeAttributes);
    }
}

TEST_CASE("Attribute Parsing - Attributes with Literal Arguments", "[parser][attributes][declarations]") {
    SECTION("@custom(\"integration\") var connection = null") {
        auto fixture = createParserFixture("@custom(\"integration\") var connection = null");
        auto *stmt = fixture->parseStatement();


        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 1);

        auto *attr = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr->name.view() == "custom");
        REQUIRE(attr->getArgCount() == 1);
        REQUIRE(attr->args[0]->kind == astString);

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [(custom "integration")]
  (Identifier connection)
  (Null)))", PrinterFlags::IncludeAttributes);
    }

    SECTION("@version(1, 2, 3) const VERSION = \"1.2.3\"") {
        auto fixture = createParserFixture("@version(1, 2, 3) const VERSION = \"1.2.3\"");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 1);

        auto *attr = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr->name.view() == "version");
        REQUIRE(attr->getArgCount() == 3);
        REQUIRE(attr->args[0]->kind == astInt);
        REQUIRE(attr->args[1]->kind == astInt);
        REQUIRE(attr->args[2]->kind == astInt);

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [(version 1 2 3)]
  (Identifier VERSION)
  (String "1.2.3")))", PrinterFlags::IncludeAttributes);
    }

    SECTION("@bounds(0.0, 100.0) var percentage = 50.0") {
        auto fixture = createParserFixture("@bounds(0.0, 100.0) var percentage = 50.0");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 1);

        auto *attr = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr->name.view() == "bounds");
        REQUIRE(attr->getArgCount() == 2);
        REQUIRE(attr->args[0]->kind == astFloat);
        REQUIRE(attr->args[1]->kind == astFloat);

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [(bounds 0 100)]
  (Identifier percentage)
  (Float 50)))", PrinterFlags::IncludeAttributes);
    }

    SECTION("@enabled(true) var feature = false") {
        auto fixture = createParserFixture("@enabled(true) var feature = false");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 1);

        auto *attr = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr->name.view() == "enabled");
        REQUIRE(attr->getArgCount() == 1);
        REQUIRE(attr->args[0]->kind == astBool);

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [(enabled true)]
  (Identifier feature)
  (Bool false)))", PrinterFlags::IncludeAttributes);
    }
}

TEST_CASE("Attribute Parsing - Attributes with Named Arguments", "[parser][attributes][declarations]") {
    SECTION("@serialize(format: \"json\") var config = null") {
        auto fixture = createParserFixture("@serialize(format: \"json\") var config = null");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 1);

        auto *attr = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr->name.view() == "serialize");
        REQUIRE(attr->getArgCount() == 1);
        REQUIRE(attr->args[0]->kind == astFieldExpr);

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [(serialize (format "json"))]
  (Identifier config)
  (Null)))", PrinterFlags::IncludeAttributes);
    }

    SECTION("@cache(ttl: 3600, strategy: \"lru\") const CACHE_CONFIG = null") {
        auto fixture = createParserFixture("@cache(ttl: 3600, strategy: \"lru\") const CACHE_CONFIG = null");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 1);

        auto *attr = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr->name.view() == "cache");
        REQUIRE(attr->getArgCount() == 2);

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [(cache (ttl 3600) (strategy "lru"))]
  (Identifier CACHE_CONFIG)
  (Null)))", PrinterFlags::IncludeAttributes);
    }

    SECTION("@validate(min: 0, max: 100, required: true) var score = 85") {
        auto fixture = createParserFixture("@validate(min: 0, max: 100, required: true) var score = 85");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 1);

        auto *attr = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr->name.view() == "validate");
        REQUIRE(attr->getArgCount() == 3);

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [(validate (min 0) (max 100) (required true))]
  (Identifier score)
  (Int 85)))", PrinterFlags::IncludeAttributes);
    }
}

TEST_CASE("Attribute Parsing - Multiple Attributes", "[parser][attributes][declarations]") {
    SECTION("@deprecated @readonly var OLD_CONSTANT = 42") {
        auto fixture = createParserFixture("@deprecated @readonly var OLD_CONSTANT = 42");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 2);

        auto *attr1 = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr1->name.view() == "deprecated");
        auto *attr2 = static_cast<AttributeNode*>(decl->getAttribute(1));
        REQUIRE(attr2->name.view() == "readonly");

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [deprecated readonly]
  (Identifier OLD_CONSTANT)
  (Int 42)))", PrinterFlags::IncludeAttributes);
    }

    SECTION("@[deprecated, since(\"1.0.0\"), readonly] const LEGACY = \"old\"") {
        auto fixture = createParserFixture("@[deprecated, since(\"1.0.0\"), readonly] const LEGACY = \"old\"");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 3);

        auto *attr1 = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr1->name.view() == "deprecated");
        auto *attr2 = static_cast<AttributeNode*>(decl->getAttribute(1));
        REQUIRE(attr2->name.view() == "since");
        auto *attr3 = static_cast<AttributeNode*>(decl->getAttribute(2));
        REQUIRE(attr3->name.view() == "readonly");

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [deprecated (since "1.0.0") readonly]
  (Identifier LEGACY)
  (String "old")))", PrinterFlags::IncludeAttributes);
    }

    SECTION("@custom(\"unit\") @benchmark @inline var fastVar = 100") {
        auto fixture = createParserFixture("@custom(\"unit\") @benchmark @inline var fastVar = 100");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 3);

        auto *attr1 = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr1->name.view() == "custom");
        REQUIRE(attr1->getArgCount() == 1);
        auto *attr2 = static_cast<AttributeNode*>(decl->getAttribute(1));
        REQUIRE(attr2->name.view() == "benchmark");
        auto *attr3 = static_cast<AttributeNode*>(decl->getAttribute(2));
        REQUIRE(attr3->name.view() == "inline");

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [(custom "unit") benchmark inline]
  (Identifier fastVar)
  (Int 100)))", PrinterFlags::IncludeAttributes);
    }
}

TEST_CASE("Attribute Parsing - Complex Attribute Arguments", "[parser][attributes][declarations]") {
    SECTION("@[cache(ttl: 3600, key: \"user_data\"), validate(min: 1)] var userData = null") {
        auto fixture = createParserFixture("@[cache(ttl: 3600, key: \"user_data\"), validate(min: 1)] var userData = null");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 2);

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [(cache (ttl 3600) (key "user_data")) (validate (min 1))]
  (Identifier userData)
  (Null)))", PrinterFlags::IncludeAttributes);
    }

    SECTION("@config(debug: true, level: 2, name: \"test\", weight: 1.5) var settings = null") {
        auto fixture = createParserFixture("@config(debug: true, level: 2, name: \"test\", weight: 1.5) var settings = null");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        auto *decl = static_cast<VariableDeclarationNode *>(stmt);
        REQUIRE(decl->hasAttributes());
        REQUIRE(decl->getAttributeCount() == 1);

        auto *attr = static_cast<AttributeNode*>(decl->getAttribute(0));
        REQUIRE(attr->name.view() == "config");
        REQUIRE(attr->getArgCount() == 4);

        REQUIRE_AST_MATCHES_FLAGS(stmt, R"((VariableDeclaration [(config (debug true) (level 2) (name "test") (weight 1.5))]
  (Identifier settings)
  (Null)))", PrinterFlags::IncludeAttributes);
    }
}

TEST_CASE("Attribute Parsing - Error Cases", "[parser][attributes][declarations][errors]") {
    SECTION("Missing attribute name after @") {
        auto fixture = createParserFixture("@ var x = 42");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing closing parenthesis in attribute arguments") {
        auto fixture = createParserFixture("@test(\"value\" var x = 42");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Invalid named argument syntax") {
        auto fixture = createParserFixture("@config({debug true}) var x = 42");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing closing bracket in attribute list") {
        auto fixture = createParserFixture("@[deprecated, readonly var x = 42");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Expression in attribute argument (not allowed)") {
        auto fixture = createParserFixture("@test(x + 1) var x = 42");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }

    SECTION("Missing variable declaration after attributes") {
        auto fixture = createParserFixture("@deprecated");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt == nullptr);
        REQUIRE(fixture->hasErrors());
    }
}

TEST_CASE("Attribute Parsing - Without Attributes Flag", "[parser][attributes][declarations]") {
    SECTION("Attributes should not appear in output without IncludeAttributes flag") {
        auto fixture = createParserFixture("@deprecated var x = 42");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        // Without IncludeAttributes flag, attributes should not appear in AST output
        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier x)
  (Int 42)))");
    }

    SECTION("Multiple attributes should not appear without flag") {
        auto fixture = createParserFixture("@deprecated @readonly var x = 42");
        auto *stmt = fixture->parseStatement();

        REQUIRE(stmt != nullptr);
        REQUIRE(stmt->kind == astVariableDeclaration);

        // Without IncludeAttributes flag, no attributes in output
        REQUIRE_AST_MATCHES(stmt, R"((VariableDeclaration
  (Identifier x)
  (Int 42)))");
    }
}
