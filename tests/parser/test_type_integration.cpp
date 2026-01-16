#include "../parser_test_utils.hpp"
#include "cxy/types/primitive.hpp"
#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser type integration: Integer literals get correct types", "[parser][types]") {
    SECTION("Integer literal with i32 suffix gets IntegerType") {
        auto fixture = createParserFixture("42i32");
        auto *node = fixture->parseLiteralExpression();
        
        REQUIRE(node != nullptr);
        REQUIRE(node->type != nullptr);
        
        // Should be an IntegerType
        auto *intType = node->type->as<IntegerType>();
        REQUIRE(intType != nullptr);
        REQUIRE(intType->getIntegerKind() == IntegerKind::I32);
        REQUIRE(intType->getBitWidth() == 32);
        REQUIRE(intType->isSigned() == true);
    }
    
    SECTION("Integer literal with u64 suffix gets correct type") {
        auto fixture = createParserFixture("123u64");
        auto *node = fixture->parseLiteralExpression();
        
        REQUIRE(node != nullptr);
        REQUIRE(node->type != nullptr);
        
        auto *intType = node->type->as<IntegerType>();
        REQUIRE(intType != nullptr);
        REQUIRE(intType->getIntegerKind() == IntegerKind::U64);
        REQUIRE(intType->getBitWidth() == 64);
        REQUIRE(intType->isSigned() == false);
    }
    
    SECTION("Integer literal with auto type inference") {
        auto fixture = createParserFixture("42");
        auto *node = fixture->parseLiteralExpression();
        
        REQUIRE(node != nullptr);
        REQUIRE(node->type != nullptr);
        
        auto *intType = node->type->as<IntegerType>();
        REQUIRE(intType != nullptr);
        REQUIRE(intType->getIntegerKind() == IntegerKind::Auto);
    }
}

TEST_CASE("Parser type integration: Float literals get correct types", "[parser][types]") {
    SECTION("Float literal with f32 suffix") {
        auto fixture = createParserFixture("3.14f");
        auto *node = fixture->parseLiteralExpression();
        
        REQUIRE(node != nullptr);
        REQUIRE(node->type != nullptr);
        
        auto *floatType = node->type->as<FloatType>();
        REQUIRE(floatType != nullptr);
        REQUIRE(floatType->getFloatKind() == FloatKind::F32);
        REQUIRE(floatType->getBitWidth() == 32);
    }
    
    SECTION("Float literal with f64 suffix (default)") {
        auto fixture = createParserFixture("2.718");
        auto *node = fixture->parseLiteralExpression();
        
        REQUIRE(node != nullptr);
        REQUIRE(node->type != nullptr);
        
        auto *floatType = node->type->as<FloatType>();
        REQUIRE(floatType != nullptr);
        REQUIRE(floatType->getFloatKind() == FloatKind::Auto);
    }
}

TEST_CASE("Parser type integration: Boolean literals get BoolType", "[parser][types]") {
    SECTION("True literal") {
        auto fixture = createParserFixture("true");
        auto *node = fixture->parseLiteralExpression();
        
        REQUIRE(node != nullptr);
        REQUIRE(node->type != nullptr);
        
        auto *boolType = node->type->as<BoolType>();
        REQUIRE(boolType != nullptr);
        REQUIRE(boolType->getStaticSize() == 1);
    }
    
    SECTION("False literal") {
        auto fixture = createParserFixture("false");
        auto *node = fixture->parseLiteralExpression();
        
        REQUIRE(node != nullptr);
        REQUIRE(node->type != nullptr);
        
        auto *boolType = node->type->as<BoolType>();
        REQUIRE(boolType != nullptr);
    }
}

TEST_CASE("Parser type integration: Character literals get CharType", "[parser][types]") {
    auto fixture = createParserFixture("'a'");
    auto *node = fixture->parseLiteralExpression();
    
    REQUIRE(node != nullptr);
    REQUIRE(node->type != nullptr);
    
    auto *charType = node->type->as<CharType>();
    REQUIRE(charType != nullptr);
    REQUIRE(charType->getStaticSize() == 4); // Unicode codepoint
}

TEST_CASE("Parser type integration: String literals get placeholder type", "[parser][types]") {
    auto fixture = createParserFixture("\"hello\"");
    auto *node = fixture->parseLiteralExpression();
    
    REQUIRE(node != nullptr);
    REQUIRE(node->type != nullptr);
    
    // Currently using CharType as placeholder for strings
    // TODO: Update when proper string type is implemented
    auto *charType = node->type->as<CharType>();
    REQUIRE(charType != nullptr);
}

TEST_CASE("Parser type integration: Expression with mixed types", "[parser][types]") {
    auto fixture = createParserFixture("42 + 3.14f");
    auto *expr = fixture->parseExpression();
    
    REQUIRE(expr != nullptr);
    REQUIRE(expr->getChildCount() == 2);
    
    // Check left operand (integer)
    auto *leftChild = expr->getChild(0);
    REQUIRE(leftChild != nullptr);
    REQUIRE(leftChild->type != nullptr);
    auto *intType = leftChild->type->as<IntegerType>();
    REQUIRE(intType != nullptr);
    REQUIRE(intType->getIntegerKind() == IntegerKind::Auto);
    
    // Check right operand (float)
    auto *rightChild = expr->getChild(1);
    REQUIRE(rightChild != nullptr);
    REQUIRE(rightChild->type != nullptr);
    auto *floatType = rightChild->type->as<FloatType>();
    REQUIRE(floatType != nullptr);
    REQUIRE(floatType->getFloatKind() == FloatKind::F32);
}

TEST_CASE("Parser type integration: Type system caching within registry", "[parser][types]") {
    // Test that types are cached within the same registry by parsing an expression
    // with two identical integer types
    auto fixture = createParserFixture("42i32 + 100i32");
    auto *expr = fixture->parseExpression();
    
    REQUIRE(expr != nullptr);
    REQUIRE(expr->getChildCount() == 2);
    
    // Check both operands
    auto *leftChild = expr->getChild(0);
    auto *rightChild = expr->getChild(1);
    
    REQUIRE(leftChild != nullptr);
    REQUIRE(rightChild != nullptr);
    REQUIRE(leftChild->type != nullptr);
    REQUIRE(rightChild->type != nullptr);
    
    // Should be the exact same type instance from the same registry
    REQUIRE(leftChild->type == rightChild->type);
    
    auto *intType1 = leftChild->type->as<IntegerType>();
    auto *intType2 = rightChild->type->as<IntegerType>();
    REQUIRE(intType1 == intType2); // Same pointer
    REQUIRE(intType1->getIntegerKind() == IntegerKind::I32);
}

TEST_CASE("Parser type integration: Type classification methods work", "[parser][types]") {
    auto fixture = createParserFixture("42i32");
    auto *node = fixture->parseLiteralExpression();
    
    REQUIRE(node != nullptr);
    REQUIRE(node->type != nullptr);
    
    const auto *type = node->type;
    
    // Test type classification
    REQUIRE(type->isPrimitive() == true);
    REQUIRE(type->isComposite() == false);
    REQUIRE(type->isNumeric() == true);
    REQUIRE(type->isIntegral() == true);
    REQUIRE(type->isFloatingPoint() == false);
    REQUIRE(type->isCallable() == false);
    REQUIRE(type->hasStaticSize() == true);
    REQUIRE(type->isDynamicallySized() == false);
}