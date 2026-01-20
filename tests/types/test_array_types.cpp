#include "catch2.hpp"
#include <cxy/types.hpp>
#include <cxy/memory/arena.hpp>
#include <cxy/ast/literals.hpp>
#include <cxy/ast/literals.hpp>

using namespace cxy;

TEST_CASE("ArrayType basic functionality", "[types][array]") {
    SECTION("ArrayType should be included in TypeKind enum") {
        // Verify that array type kind exists
        REQUIRE(static_cast<int>(typArray) != 0);
    }
    
    SECTION("ArrayType creation with TypeRegistry") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        // Test ArrayType creation via TypeRegistry
        auto arrayType = registry.getArrayType(i32, 10);
        REQUIRE(arrayType != nullptr);
        REQUIRE(arrayType->getElementType() == i32);
        REQUIRE(arrayType->getArraySize() == 10);
        REQUIRE(arrayType->isFixedArray() == true);
        REQUIRE(arrayType->isDynamicArray() == false);
    }
}

TEST_CASE("ArrayType implementation", "[types][array][implementation]") {
    SECTION("ArrayType inherits from CompositeType") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto arrayType = registry.getArrayType(i32, 10);
        
        // Test inheritance and type classification
        REQUIRE(arrayType->isPrimitive() == false);
        REQUIRE(arrayType->isComposite() == true);
        REQUIRE(arrayType->kind() == typArray);
        
        // Test CompositeType casting
        auto composite = arrayType->as<CompositeType>();
        REQUIRE(composite != nullptr);
    }
    
    SECTION("ArrayType size calculations") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        
        // Fixed array: [10]i32 should be 40 bytes (4 * 10)
        auto fixedArray = registry.getArrayType(i32, 10);
        REQUIRE(fixedArray->getStaticSize() == 40);
        REQUIRE(fixedArray->getAlignment() == 4);
        REQUIRE(fixedArray->hasStaticSize() == true);
        REQUIRE(fixedArray->isDynamicallySized() == false);
        
        // Dynamic array: []i32 should have pointer size
        auto dynamicArray = registry.getArrayType(i32, 0);
        REQUIRE(dynamicArray->getStaticSize() == sizeof(void*));
        REQUIRE(dynamicArray->getAlignment() == sizeof(void*));
        REQUIRE(dynamicArray->hasStaticSize() == true);
        REQUIRE(dynamicArray->isDynamicallySized() == false);
    }
    
    SECTION("ArrayType equality and hashing") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        
        // Two arrays with same element type and size should be equal
        auto array1 = registry.getArrayType(i32, 10);
        auto array2 = registry.getArrayType(i32, 10);
        auto array3 = registry.getArrayType(i32, 5);
        auto array4 = registry.getArrayType(f64, 10);
        
        REQUIRE(array1 == array2); // Same instance from cache
        REQUIRE(array1->equals(array2));
        REQUIRE_FALSE(array1->equals(array3)); // Different size
        REQUIRE_FALSE(array1->equals(array4)); // Different element type
        
        // Hash should be consistent
        REQUIRE(array1->hash() == array2->hash());
    }
}

TEST_CASE("ArrayType usage scenarios", "[types][array][usage]") {
    SECTION("Fixed size arrays") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        
        // [10]i32
        auto intArray = registry.getArrayType(i32, 10);
        REQUIRE(intArray->toString() == "[10]i32");
        REQUIRE(intArray->isFixedArray() == true);
        REQUIRE(intArray->getStaticSize() == 40);
        
        // [5]f64
        auto floatArray = registry.getArrayType(f64, 5);
        REQUIRE(floatArray->toString() == "[5]f64");
        REQUIRE(floatArray->getStaticSize() == 40); // 8 * 5
    }
    
    SECTION("Dynamic arrays") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        // []i32
        auto dynamicArray = registry.getArrayType(i32, 0);
        REQUIRE(dynamicArray->toString() == "[]i32");
        REQUIRE(dynamicArray->isDynamicArray() == true);
        REQUIRE(dynamicArray->getStaticSize() == sizeof(void*));
    }
    
    SECTION("Nested arrays") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        // [4]i32 (inner array)
        auto innerArray = registry.getArrayType(i32, 4);
        REQUIRE(innerArray->getStaticSize() == 16); // 4 * 4
        
        // [3][4]i32 (3 arrays of 4 integers each)
        auto outerArray = registry.getArrayType(innerArray, 3);
        REQUIRE(outerArray->toString() == "[3][4]i32");
        REQUIRE(outerArray->getStaticSize() == 48); // 16 * 3
    }
    
    SECTION("Direct API access") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        // Test direct access to array properties
        auto fixedArray = registry.getArrayType(i32, 10);
        REQUIRE(fixedArray->getArraySize() == 10);
        REQUIRE(fixedArray->isFixedArray() == true);
        REQUIRE(fixedArray->isDynamicArray() == false);
        REQUIRE(fixedArray->getElementType() == i32);
        
        // Test dynamic array direct access
        auto dynamicArray = registry.getArrayType(i32, 0);
        REQUIRE(dynamicArray->getArraySize() == 0);
        REQUIRE(dynamicArray->isFixedArray() == false);
        REQUIRE(dynamicArray->isDynamicArray() == true);
        REQUIRE(dynamicArray->getElementType() == i32);
    }
}