#include "catch2.hpp"
#include <cxy/types.hpp>
#include <set>

using namespace cxy;

TEST_CASE("Integer type creation and properties", "[types][primitive][integer]") {
    SECTION("I32 type properties") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(::cxy::IntegerKind::I32);
        REQUIRE(i32 != nullptr);
        REQUIRE(i32->getStaticSize() == 4);
        REQUIRE(i32->getAlignment() == 4);
        REQUIRE(i32->isSigned() == true);
        REQUIRE(i32->getBitWidth() == 32);
        REQUIRE(i32->toString() == "i32");
        REQUIRE(i32->kind() == typInteger);
        REQUIRE(i32->isNumeric() == true);
        REQUIRE(i32->isIntegral() == true);
        REQUIRE(i32->isFloatingPoint() == false);
        REQUIRE(i32->isPrimitive() == true);
        REQUIRE(i32->isComposite() == false);
    }

    SECTION("U64 type properties") {
        auto& registry = TypeRegistry::instance();
        auto u64 = registry.integerType(::cxy::IntegerKind::U64);
        REQUIRE(u64 != nullptr);
        REQUIRE(u64->getStaticSize() == 8);
        REQUIRE(u64->getAlignment() == 8);
        REQUIRE(u64->isSigned() == false);
        REQUIRE(u64->getBitWidth() == 64);
        REQUIRE(u64->toString() == "u64");
    }

    SECTION("All integer types have correct sizes") {
        auto& registry = TypeRegistry::instance();
        REQUIRE(registry.integerType(::cxy::IntegerKind::I8)->getStaticSize() == 1);
        REQUIRE(registry.integerType(::cxy::IntegerKind::I16)->getStaticSize() == 2);
        REQUIRE(registry.integerType(::cxy::IntegerKind::I32)->getStaticSize() == 4);
        REQUIRE(registry.integerType(::cxy::IntegerKind::I64)->getStaticSize() == 8);
        REQUIRE(registry.integerType(::cxy::IntegerKind::I128)->getStaticSize() == 16);
        REQUIRE(registry.integerType(::cxy::IntegerKind::U8)->getStaticSize() == 1);
        REQUIRE(registry.integerType(::cxy::IntegerKind::U16)->getStaticSize() == 2);
        REQUIRE(registry.integerType(::cxy::IntegerKind::U32)->getStaticSize() == 4);
        REQUIRE(registry.integerType(::cxy::IntegerKind::U64)->getStaticSize() == 8);
        REQUIRE(registry.integerType(::cxy::IntegerKind::U128)->getStaticSize() == 16);
    }
}

TEST_CASE("Float type creation and properties", "[types][primitive][float]") {
    SECTION("F64 type properties") {
        auto& registry = TypeRegistry::instance();
        auto f64 = registry.floatType(::cxy::FloatKind::F64);
        REQUIRE(f64 != nullptr);
        REQUIRE(f64->getStaticSize() == 8);
        REQUIRE(f64->getBitWidth() == 64);
        REQUIRE(f64->toString() == "f64");
        REQUIRE(f64->kind() == typFloat);
        REQUIRE(f64->isNumeric() == true);
        REQUIRE(f64->isIntegral() == false);
        REQUIRE(f64->isFloatingPoint() == true);
        REQUIRE(f64->isPrimitive() == true);
        REQUIRE(f64->isComposite() == false);
    }

    SECTION("F32 type properties") {
        auto& registry = TypeRegistry::instance();
        auto f32 = registry.floatType(::cxy::FloatKind::F32);
        REQUIRE(f32 != nullptr);
        REQUIRE(f32->getStaticSize() == 4);
        REQUIRE(f32->getBitWidth() == 32);
        REQUIRE(f32->toString() == "f32");
    }

    SECTION("Float types have expected ranges") {
        auto& registry = TypeRegistry::instance();
        auto f32 = registry.floatType(::cxy::FloatKind::F32);
        auto f64 = registry.floatType(::cxy::FloatKind::F64);
        
        REQUIRE(f32->getEpsilon() > 0);
        REQUIRE(f64->getEpsilon() > 0);
        REQUIRE(f32->getEpsilon() > f64->getEpsilon()); // F32 has larger epsilon
        REQUIRE(f32->getMaxValue() > 0);
        REQUIRE(f32->getMinValue() < 0);
        REQUIRE(f64->getMaxValue() > f32->getMaxValue());
    }
}

TEST_CASE("Type equality and comparison", "[types][primitive][equality]") {
    SECTION("Integer type singleton pattern") {
        auto& registry = TypeRegistry::instance();
        auto i32a = registry.integerType(::cxy::IntegerKind::I32);
        auto i32b = registry.integerType(::cxy::IntegerKind::I32);
        auto i64 = registry.integerType(::cxy::IntegerKind::I64);

        REQUIRE(i32a == i32b);  // Singleton pattern - same pointer
        REQUIRE(i32a->equals(i32b));
        REQUIRE_FALSE(i32a->equals(i64));
        REQUIRE(i32a != i64);
    }

    SECTION("Float type singleton pattern") {
        auto& registry = TypeRegistry::instance();
        auto f32a = registry.floatType(::cxy::FloatKind::F32);
        auto f32b = registry.floatType(::cxy::FloatKind::F32);
        auto f64 = registry.floatType(::cxy::FloatKind::F64);

        REQUIRE(f32a == f32b);
        REQUIRE(f32a->equals(f32b));
        REQUIRE_FALSE(f32a->equals(f64));
    }

    SECTION("Other primitive types singleton") {
        auto& registry = TypeRegistry::instance();
        auto bool1 = registry.boolType();
        auto bool2 = registry.boolType();
        auto char1 = registry.charType();
        auto char2 = registry.charType();
        auto void1 = registry.voidType();
        auto void2 = registry.voidType();
        auto auto1 = registry.autoType();
        auto auto2 = registry.autoType();

        REQUIRE(bool1 == bool2);
        REQUIRE(char1 == char2);
        REQUIRE(void1 == void2);
        REQUIRE(auto1 == auto2);
    }
}

TEST_CASE("Best fit type selection", "[types][primitive][bestfit]") {
    SECTION("Integer best fit selection") {
        auto smallInt = types::findBestIntegerType(42, true);
        REQUIRE(smallInt->getIntegerKind() == ::cxy::IntegerKind::I8);

        auto largeInt = types::findBestIntegerType(1000, true);
        REQUIRE(largeInt->getIntegerKind() == ::cxy::IntegerKind::I16);

        auto veryLargeInt = types::findBestIntegerType(100000, true);
        REQUIRE(veryLargeInt->getIntegerKind() == ::cxy::IntegerKind::I32);

        // Unsigned selection
        auto smallUnsigned = types::findBestIntegerType(200, false);
        REQUIRE(smallUnsigned->getIntegerKind() == ::cxy::IntegerKind::U8);

        auto largeUnsigned = types::findBestIntegerType(70000, false);
        REQUIRE(largeUnsigned->getIntegerKind() == ::cxy::IntegerKind::U32);
    }

    SECTION("Float best fit selection") {
        // Value that fits in F32
        auto smallFloat = types::findBestFloatType(3.14f);
        REQUIRE(smallFloat->getFloatKind() == ::cxy::FloatKind::F32);

        // Value that requires F64 precision
        auto preciseFloat = types::findBestFloatType(3.141592653589793);
        REQUIRE(preciseFloat->getFloatKind() == ::cxy::FloatKind::F64);
    }
}

TEST_CASE("Bool, Char, Void, Auto type properties", "[types][primitive][other]") {
    SECTION("Bool type") {
        auto& registry = TypeRegistry::instance();
        auto boolType = registry.boolType();
        REQUIRE(boolType->kind() == typBool);
        REQUIRE(boolType->getStaticSize() == 1);
        REQUIRE(boolType->getAlignment() == 1);
        REQUIRE(boolType->toString() == "bool");
        REQUIRE_FALSE(boolType->isNumeric());
        REQUIRE_FALSE(boolType->isIntegral());
        REQUIRE_FALSE(boolType->isFloatingPoint());
        REQUIRE(boolType->isPrimitive());
        REQUIRE_FALSE(boolType->isComposite());
    }

    SECTION("Char type") {
        auto& registry = TypeRegistry::instance();
        auto charType = registry.charType();
        REQUIRE(charType->kind() == typChar);
        REQUIRE(charType->getStaticSize() == 4); // UTF-32
        REQUIRE(charType->getAlignment() == 4);
        REQUIRE(charType->toString() == "char");
        REQUIRE_FALSE(charType->isNumeric());
        REQUIRE_FALSE(charType->isIntegral());
        REQUIRE_FALSE(charType->isFloatingPoint());
    }

    SECTION("Void type") {
        auto& registry = TypeRegistry::instance();
        auto voidType = registry.voidType();
        REQUIRE(voidType->kind() == typVoid);
        REQUIRE(voidType->getStaticSize() == 0);
        REQUIRE(voidType->getAlignment() == 1);
        REQUIRE(voidType->toString() == "void");
        REQUIRE_FALSE(voidType->hasStaticSize());
        REQUIRE_FALSE(voidType->isNumeric());
    }

    SECTION("Auto type") {
        auto& registry = TypeRegistry::instance();
        auto autoType = registry.autoType();
        REQUIRE(autoType->kind() == typAuto);
        REQUIRE(autoType->getStaticSize() == 0);
        REQUIRE(autoType->getAlignment() == 1);
        REQUIRE(autoType->toString() == "auto");
        REQUIRE_FALSE(autoType->hasStaticSize());
        REQUIRE(autoType->isDynamicallySized());
        REQUIRE_FALSE(autoType->isResolved());
        REQUIRE(autoType->getResolvedType() == nullptr);
    }
}

TEST_CASE("Type conversion rules", "[types][primitive][conversion]") {
    SECTION("Integer implicit conversions") {
        auto& registry = TypeRegistry::instance();
        auto i8 = registry.integerType(::cxy::IntegerKind::I8);
        auto i16 = registry.integerType(::cxy::IntegerKind::I16);
        auto i32 = registry.integerType(::cxy::IntegerKind::I32);
        auto u8 = registry.integerType(::cxy::IntegerKind::U8);
        auto u16 = registry.integerType(::cxy::IntegerKind::U16);

        // Smaller to larger implicit conversion
        REQUIRE(i8->isImplicitlyConvertibleTo(i16));
        REQUIRE(i8->isImplicitlyConvertibleTo(i32));
        REQUIRE(i16->isImplicitlyConvertibleTo(i32));
        
        // Larger to smaller requires explicit conversion
        REQUIRE_FALSE(i32->isImplicitlyConvertibleTo(i16));
        REQUIRE(i32->isExplicitlyConvertibleTo(i16));
    }

    SECTION("Float implicit conversions") {
        auto& registry = TypeRegistry::instance();
        auto f32 = registry.floatType(::cxy::FloatKind::F32);
        auto f64 = registry.floatType(::cxy::FloatKind::F64);

        // F32 can convert to F64 implicitly
        REQUIRE(f32->isImplicitlyConvertibleTo(f64));
        
        // F64 to F32 requires explicit conversion
        REQUIRE_FALSE(f64->isImplicitlyConvertibleTo(f32));
        REQUIRE(f64->isExplicitlyConvertibleTo(f32));
    }

    SECTION("Cross-type conversions") {
        auto& registry = TypeRegistry::instance();
        auto boolType = registry.boolType();
        auto charType = registry.charType();
        auto i32 = registry.integerType(::cxy::IntegerKind::I32);

        // Bool to integer explicit conversion
        REQUIRE_FALSE(boolType->isImplicitlyConvertibleTo(i32));
        REQUIRE(boolType->isExplicitlyConvertibleTo(i32));

        // Char to integer implicit conversion
        REQUIRE(charType->isImplicitlyConvertibleTo(i32));
    }
}

TEST_CASE("TypeRegistry integration", "[types][primitive][registry]") {
    SECTION("Registry provides consistent instances") {
        auto& registry = TypeRegistry::instance();
        
        // Test that multiple calls return same instance
        auto i32a = registry.integerType(::cxy::IntegerKind::I32);
        auto i32b = registry.integerType(::cxy::IntegerKind::I32);
        REQUIRE(i32a == i32b);

        auto f64a = registry.floatType(::cxy::FloatKind::F64);
        auto f64b = registry.floatType(::cxy::FloatKind::F64);
        REQUIRE(f64a == f64b);

        auto bool1 = registry.boolType();
        auto bool2 = registry.boolType();
        REQUIRE(bool1 == bool2);

        auto char1 = registry.charType();
        auto char2 = registry.charType();
        REQUIRE(char1 == char2);

        auto void1 = registry.voidType();
        auto void2 = registry.voidType();
        REQUIRE(void1 == void2);

        auto auto1 = registry.autoType();
        auto auto2 = registry.autoType();
        REQUIRE(auto1 == auto2);
    }
}

TEST_CASE("Type utility functions", "[types][primitive][utilities]") {
    SECTION("findBestIntegerType utility") {
        auto i8Type = types::findBestIntegerType(42, true);
        REQUIRE(i8Type->getIntegerKind() == ::cxy::IntegerKind::I8);

        auto u16Type = types::findBestIntegerType(65000, false);
        REQUIRE(u16Type->getIntegerKind() == ::cxy::IntegerKind::U16);
    }

    SECTION("findBestFloatType utility") {
        auto f32Type = types::findBestFloatType(3.14f);
        REQUIRE(f32Type->getFloatKind() == ::cxy::FloatKind::F32);
    }

    SECTION("valueCanFitIn utility") {
        auto& registry = TypeRegistry::instance();
        auto i8 = registry.integerType(::cxy::IntegerKind::I8);
        REQUIRE(types::valueCanFitIn(100, true, i8));
        REQUIRE_FALSE(types::valueCanFitIn(1000, true, i8));
    }

    SECTION("floatCanFitInF32 utility") {
        REQUIRE(types::floatCanFitInF32(3.14f));
        REQUIRE_FALSE(types::floatCanFitInF32(3.141592653589793)); // Needs F64 precision
    }

    SECTION("Binary operation promotion") {
        auto& registry = TypeRegistry::instance();
        auto i8 = registry.integerType(::cxy::IntegerKind::I8);
        auto i32 = registry.integerType(::cxy::IntegerKind::I32);
        
        auto promoted = types::promoteForBinaryOperation(i8, i32);
        REQUIRE(promoted == i32); // Promote to larger type
        
        auto f32 = registry.floatType(::cxy::FloatKind::F32);
        auto mixedPromotion = types::promoteForBinaryOperation(i32, f32);
        REQUIRE(mixedPromotion == f32); // Promote integer to float
    }

    SECTION("canImplicitlyConvert utility") {
        auto& registry = TypeRegistry::instance();
        auto i8 = registry.integerType(::cxy::IntegerKind::I8);
        auto i16 = registry.integerType(::cxy::IntegerKind::I16);
        auto u8 = registry.integerType(::cxy::IntegerKind::U8);

        REQUIRE(types::canImplicitlyConvert(i8, i16));
        REQUIRE_FALSE(types::canImplicitlyConvert(i16, i8));
    }
}

TEST_CASE("Type hash and equality for containers", "[types][primitive][hash]") {
    SECTION("Types can be used in hash containers") {
        auto& registry = TypeRegistry::instance();
        std::set<const Type*> typeSet;
        typeSet.insert(registry.integerType(::cxy::IntegerKind::I32));
        typeSet.insert(registry.floatType(::cxy::FloatKind::F64));
        typeSet.insert(registry.boolType());
        typeSet.insert(registry.charType());
        typeSet.insert(registry.voidType());
        typeSet.insert(registry.autoType());

        REQUIRE(typeSet.size() == 6);
    }

    SECTION("TypeHash and TypeEqual work correctly") {
        auto& registry = TypeRegistry::instance();
        TypeHash hasher;
        TypeEqual equalizer;

        auto i32 = registry.integerType(::cxy::IntegerKind::I32);
        auto f64 = registry.floatType(::cxy::FloatKind::F64);

        REQUIRE(hasher(i32) != 0);
        REQUIRE(hasher(i32) == hasher(i32)); // Same type, same hash
        REQUIRE(hasher(i32) != hasher(f64)); // Different types, different hash

        REQUIRE(equalizer(i32, i32));
        REQUIRE_FALSE(equalizer(i32, f64));
    }
}