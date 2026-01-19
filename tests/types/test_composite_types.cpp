#include "catch2.hpp"
#include <cxy/types.hpp>
#include <cxy/ast/node.hpp>
#include <cxy/ast/literals.hpp>

#include <cxy/arena_allocator.hpp>
#include <cxy/strings.hpp>

using namespace cxy;

// Test implementation of CompositeType for testing
class TestCompositeType : public CompositeType {
public:
    explicit TestCompositeType(const ast::ASTNode* ast, Flags flags = flgNone)
        : CompositeType(ast, flags) {}

    // Type interface implementation
    TypeKind kind() const override { return typStruct; }

    bool equals(const Type* other) const override {
        auto otherTest = dynamic_cast<const TestCompositeType*>(other);
        return otherTest && getSourceAST() == otherTest->getSourceAST();
    }

    std::string toString() const override {
        return "TestCompositeType";
    }

    size_t hash() const override {
        return std::hash<const void*>{}(getSourceAST());
    }

    // Type relationship queries - simple implementation for testing
    bool isAssignableFrom(const Type* other) const override { return false; }
    bool isImplicitlyConvertibleTo(const Type* other) const override { return false; }
    bool isExplicitlyConvertibleTo(const Type* other) const override { return false; }
    bool isCompatibleWith(const Type* other) const override { return false; }

    // Size and alignment - test values
    size_t getStaticSize() const override { return 8; }
    size_t getAlignment() const override { return 8; }
    bool hasStaticSize() const override { return true; }
    bool isDynamicallySized() const override { return false; }

    // Type classification - inherited from CompositeType
    bool isCallable() const override { return false; }
    bool isNumeric() const override { return false; }
    bool isIntegral() const override { return false; }
    bool isFloatingPoint() const override { return false; }


};

// Mock AST node for testing
class MockAST : public ast::ASTNode {
public:
    MockAST(ArenaAllocator& arena)
        : ast::ASTNode(ast::astStructDeclaration, Location{}, arena) {}

    std::format_context::iterator toString(std::format_context& ctx) const override {
        return std::format_to(ctx.out(), "MockAST");
    }
};

TEST_CASE("CompositeType basic functionality", "[types][composite]") {
    ArenaAllocator arena(1024);

    SECTION("CompositeType should be included in TypeKind enum") {
        // Verify that composite type kinds exist
        REQUIRE(static_cast<int>(typArray) != 0);
        REQUIRE(static_cast<int>(typStruct) != 0);
        REQUIRE(static_cast<int>(typClass) != 0);
        REQUIRE(static_cast<int>(typTuple) != 0);
    }

    SECTION("Type classification methods work for primitive vs composite") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);

        auto mockAST = arena.construct<MockAST>(arena);
        TestCompositeType compositeType(mockAST);

        // Primitive type classification
        REQUIRE(i32->isPrimitive() == true);
        REQUIRE(i32->isComposite() == false);

        // Composite type classification
        REQUIRE(compositeType.isPrimitive() == false);
        REQUIRE(compositeType.isComposite() == true);
    }

    SECTION("CompositeType AST integration") {
        auto mockAST = arena.construct<MockAST>(arena);
        TestCompositeType compositeType(mockAST);

        REQUIRE(compositeType.getSourceAST() == mockAST);
    }

    SECTION("CompositeType flags integration") {
        auto mockAST = arena.construct<MockAST>(arena);

        TestCompositeType normalType(mockAST, flgNone);
        TestCompositeType constType(mockAST, flgConst);
        TestCompositeType publicType(mockAST, flgPublic);

        REQUIRE(normalType.getFlags() == flgNone);
        REQUIRE(constType.getFlags() == flgConst);
        REQUIRE(publicType.getFlags() == flgPublic);

        REQUIRE(normalType.hasFlag(flgConst) == false);
        REQUIRE(constType.hasFlag(flgConst) == true);
        REQUIRE(publicType.hasFlag(flgPublic) == true);
    }
}

TEST_CASE("Phase 3 Completion Verification", "[types][composite][phase3]") {
    SECTION("Phase 3 acceptance criteria verification") {
        ArenaAllocator arena(1024);
        auto mockAST = arena.construct<MockAST>(arena);

        // ✓ CompositeType base class with AST reference
        TestCompositeType compositeType(mockAST, flgConst);
        REQUIRE(compositeType.getSourceAST() == mockAST);

        // ✓ Type hierarchy working with proper casting

        // ✓ Type hierarchy working with proper casting
        auto& registry = TypeRegistry::instance();
        auto primitive = registry.integerType(IntegerKind::I32);
        REQUIRE(primitive->as<CompositeType>() == nullptr);
        REQUIRE(compositeType.as<CompositeType>() != nullptr);

        // ✓ Flags integration working
        REQUIRE(compositeType.hasFlag(flgConst) == true);
        REQUIRE(compositeType.getFlags() == flgConst);

        // ✓ Type classification methods working
        REQUIRE(compositeType.isPrimitive() == false);
        REQUIRE(compositeType.isComposite() == true);

        INFO("Phase 3 (CompositeType foundation) completed successfully!");
        INFO("Ready for Phase 4 (StructType implementation)");
    }
}

TEST_CASE("CompositeType implementation", "[types][composite][implementation]") {
    ArenaAllocator arena(1024);

    SECTION("CompositeType basic functionality") {
        auto mockAST = arena.construct<MockAST>(arena);
        TestCompositeType compositeType(mockAST);

        // Test basic type classification
        REQUIRE(compositeType.isPrimitive() == false);
        REQUIRE(compositeType.isComposite() == true);
        REQUIRE(compositeType.getSourceAST() == mockAST);
    }

    SECTION("CompositeType equality and hashing") {
        auto mockAST1 = arena.construct<MockAST>(arena);
        auto mockAST2 = arena.construct<MockAST>(arena);

        TestCompositeType type1(mockAST1);
        TestCompositeType type1Copy(mockAST1);
        TestCompositeType type2(mockAST2);

        // Equality based on AST pointer
        REQUIRE(type1.equals(&type1Copy) == true);
        REQUIRE(type1.equals(&type2) == false);

        // Hash should be consistent
        REQUIRE(type1.hash() == type1Copy.hash());
    }

    SECTION("CompositeType type casting") {
        auto& registry = TypeRegistry::instance();
        auto primitive = registry.integerType(IntegerKind::I32);

        auto mockAST = arena.construct<MockAST>(arena);
        TestCompositeType composite(mockAST);

        // Test dynamic casting
        REQUIRE(primitive->as<CompositeType>() == nullptr);
        REQUIRE(composite.as<CompositeType>() != nullptr);
        REQUIRE(composite.as<TestCompositeType>() != nullptr);
    }
}

TEST_CASE("TupleType basic functionality", "[types][tuple]") {
    SECTION("TupleType should be included in TypeKind enum") {
        // Verify that tuple type kind exists
        REQUIRE(static_cast<int>(typTuple) != 0);
    }

    SECTION("TupleType creation with TypeRegistry") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        // Create element types vector
        auto elementTypes = makeArenaVector<const Type*>(arena);
        elementTypes.push_back(i32);
        elementTypes.push_back(f64);
        elementTypes.push_back(boolType);

        // Test TupleType creation via TypeRegistry
        auto tupleType = registry.getTupleType(elementTypes);
        REQUIRE(tupleType != nullptr);
        REQUIRE(tupleType->getElementCount() == 3);
        REQUIRE(tupleType->getElementType(0) == i32);
        REQUIRE(tupleType->getElementType(1) == f64);
        REQUIRE(tupleType->getElementType(2) == boolType);
        REQUIRE(tupleType->getElementType(3) == nullptr); // Out of bounds
    }
}

TEST_CASE("TupleType implementation", "[types][tuple][implementation]") {
    SECTION("TupleType inherits from CompositeType") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);

        auto elementTypes = makeArenaVector<const Type*>(arena);
        elementTypes.push_back(i32);
        elementTypes.push_back(f64);

        auto tupleType = registry.getTupleType(elementTypes);

        // Test inheritance and type classification
        REQUIRE(tupleType->isPrimitive() == false);
        REQUIRE(tupleType->isComposite() == true);
        REQUIRE(tupleType->kind() == typTuple);

        // Test CompositeType casting
        auto composite = tupleType->as<CompositeType>();
        REQUIRE(composite != nullptr);
    }

    SECTION("TupleType size calculations") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);  // 4 bytes, align 4
        auto f64 = registry.floatType(FloatKind::F64);      // 8 bytes, align 8
        auto boolType = registry.boolType();                // 1 byte, align 1

        // Tuple: (i32, f64, bool)
        // Layout: [i32:4] [padding:4] [f64:8] [bool:1] [padding:7] = 24 bytes
        // Alignment: max(4, 8, 1) = 8
        auto elementTypes = makeArenaVector<const Type*>(arena);
        elementTypes.push_back(i32);
        elementTypes.push_back(f64);
        elementTypes.push_back(boolType);

        auto tupleType = registry.getTupleType(elementTypes);
        REQUIRE(tupleType->getAlignment() == 8);
        REQUIRE(tupleType->getStaticSize() == 24);
        REQUIRE(tupleType->hasStaticSize() == true);
        REQUIRE(tupleType->isDynamicallySized() == false);
    }

    SECTION("TupleType equality and hashing") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        // Two tuples with same element types should be equal
        auto elementTypes1 = makeArenaVector<const Type*>(arena);
        elementTypes1.push_back(i32);
        elementTypes1.push_back(f64);

        auto elementTypes2 = makeArenaVector<const Type*>(arena);
        elementTypes2.push_back(i32);
        elementTypes2.push_back(f64);

        auto elementTypes3 = makeArenaVector<const Type*>(arena);
        elementTypes3.push_back(f64);
        elementTypes3.push_back(i32);  // Different order

        auto elementTypes4 = makeArenaVector<const Type*>(arena);
        elementTypes4.push_back(i32);
        elementTypes4.push_back(f64);
        elementTypes4.push_back(boolType);  // Different count

        auto tuple1 = registry.getTupleType(elementTypes1);
        auto tuple2 = registry.getTupleType(elementTypes2);
        auto tuple3 = registry.getTupleType(elementTypes3);
        auto tuple4 = registry.getTupleType(elementTypes4);

        REQUIRE(tuple1 == tuple2); // Same instance from cache
        REQUIRE(tuple1->equals(tuple2));
        REQUIRE_FALSE(tuple1->equals(tuple3)); // Different order
        REQUIRE_FALSE(tuple1->equals(tuple4)); // Different count

        // Hash should be consistent
        REQUIRE(tuple1->hash() == tuple2->hash());
    }
}

TEST_CASE("TupleType usage scenarios", "[types][tuple][usage]") {
    SECTION("Single element tuple") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);

        auto elementTypes = makeArenaVector<const Type*>(arena);
        elementTypes.push_back(i32);

        auto tupleType = registry.getTupleType(elementTypes);
        REQUIRE(tupleType->toString() == "(i32)");
        REQUIRE(tupleType->getElementCount() == 1);
        REQUIRE(tupleType->getStaticSize() == 4);
    }

    SECTION("Multi-element tuples") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        auto elementTypes = makeArenaVector<const Type*>(arena);
        elementTypes.push_back(i32);
        elementTypes.push_back(f64);
        elementTypes.push_back(boolType);

        auto tupleType = registry.getTupleType(elementTypes);
        REQUIRE(tupleType->toString() == "(i32, f64, bool)");
        REQUIRE(tupleType->getElementCount() == 3);
    }

    SECTION("Nested tuples") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);

        // Inner tuple (i32, f64)
        auto innerElements = makeArenaVector<const Type*>(arena);
        innerElements.push_back(i32);
        innerElements.push_back(f64);
        auto innerTuple = registry.getTupleType(innerElements);

        // Outer tuple ((i32, f64), i32)
        auto outerElements = makeArenaVector<const Type*>(arena);
        outerElements.push_back(innerTuple);
        outerElements.push_back(i32);
        auto outerTuple = registry.getTupleType(outerElements);

        REQUIRE(outerTuple->toString() == "((i32, f64), i32)");
        REQUIRE(outerTuple->getElementCount() == 2);
        REQUIRE(outerTuple->getElementType(0) == innerTuple);
    }

    SECTION("Compile-time properties") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        auto elementTypes = makeArenaVector<const Type*>(arena);
        elementTypes.push_back(i32);
        elementTypes.push_back(f64);
        elementTypes.push_back(boolType);

        auto tupleType = registry.getTupleType(elementTypes);

        // TupleType tests complete - compile-time properties removed
    }
}

TEST_CASE("TupleType Phase 4 Completion Verification", "[types][tuple][phase4]") {
    SECTION("TupleType acceptance criteria verification") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        // ✓ TupleType class implementation
        auto elementTypes = makeArenaVector<const Type*>(arena);
        elementTypes.push_back(i32);
        elementTypes.push_back(f64);
        elementTypes.push_back(boolType);

        auto tupleType = registry.getTupleType(elementTypes);
        REQUIRE(tupleType != nullptr);
        REQUIRE(tupleType->kind() == typTuple);

        // ✓ ArenaVector storage for element types
        REQUIRE(tupleType->getElementCount() == 3);
        REQUIRE(tupleType->getElementType(0) == i32);
        REQUIRE(tupleType->getElementType(1) == f64);
        REQUIRE(tupleType->getElementType(2) == boolType);

        // ✓ Size and alignment calculations with struct layout rules
        REQUIRE(tupleType->getAlignment() == 8); // max(4, 8, 1)
        REQUIRE(tupleType->getStaticSize() == 24); // [i32:4][pad:4][f64:8][bool:1][pad:7]
        REQUIRE(tupleType->hasStaticSize() == true);
        REQUIRE(tupleType->isDynamicallySized() == false);

        // ✓ Type classification methods working
        REQUIRE(tupleType->isPrimitive() == false);
        REQUIRE(tupleType->isComposite() == true);

        // ✓ TypeRegistry caching
        auto tupleType2 = registry.getTupleType(elementTypes);
        REQUIRE(tupleType == tupleType2); // Same instance from cache

        // ✓ Type equality and hashing
        auto otherElements = makeArenaVector<const Type*>(arena);
        otherElements.push_back(i32);
        otherElements.push_back(f64);
        otherElements.push_back(boolType);
        auto tupleType3 = registry.getTupleType(otherElements);
        REQUIRE(tupleType->equals(tupleType3));
        REQUIRE(tupleType->hash() == tupleType3->hash());

        // ✓ String representation
        REQUIRE(tupleType->toString() == "(i32, f64, bool)");

        // ✓ Inheritance from CompositeType
        REQUIRE(tupleType->isPrimitive() == false);
        REQUIRE(tupleType->isComposite() == true);
        auto composite = tupleType->as<CompositeType>();
        REQUIRE(composite != nullptr);

        INFO("Phase 4 (TupleType implementation) completed successfully!");
        INFO("Ready for Phase 5 (StructType implementation)");
    }
}



TEST_CASE("UnionType basic functionality", "[types][union]") {
    SECTION("UnionType should be included in TypeKind enum") {
        // Verify that union type kind exists
        REQUIRE(static_cast<int>(typUnion) != 0);
    }

    SECTION("UnionType creation with TypeRegistry") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        // Create variant types vector
        auto variantTypes = makeArenaVector<const Type*>(arena);
        variantTypes.push_back(i32);
        variantTypes.push_back(f64);
        variantTypes.push_back(boolType);

        // Test UnionType creation via TypeRegistry
        auto unionType = registry.getUnionType(variantTypes);
        REQUIRE(unionType != nullptr);
        REQUIRE(unionType->getVariantCount() == 3);
        REQUIRE(unionType->getVariantType(0) == i32);
        REQUIRE(unionType->getVariantType(1) == f64);
        REQUIRE(unionType->getVariantType(2) == boolType);
        REQUIRE(unionType->getVariantType(3) == nullptr); // Out of bounds
    }
}

TEST_CASE("UnionType implementation", "[types][union][implementation]") {
    SECTION("UnionType inherits from CompositeType") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);

        auto variantTypes = makeArenaVector<const Type*>(arena);
        variantTypes.push_back(i32);
        variantTypes.push_back(f64);

        auto unionType = registry.getUnionType(variantTypes);

        // Test inheritance and type classification
        REQUIRE(unionType->isPrimitive() == false);
        REQUIRE(unionType->isComposite() == true);
        REQUIRE(unionType->kind() == typUnion);

        // Test CompositeType casting
        auto composite = unionType->as<CompositeType>();
        REQUIRE(composite != nullptr);
    }

    SECTION("UnionType size calculations") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);  // 4 bytes, align 4
        auto f64 = registry.floatType(FloatKind::F64);      // 8 bytes, align 8
        auto boolType = registry.boolType();                // 1 byte, align 1

        // Union: (i32 | f64 | bool)
        // Size: max(4, 8, 1) = 8 bytes
        // Alignment: max(4, 8, 1) = 8
        auto variantTypes = makeArenaVector<const Type*>(arena);
        variantTypes.push_back(i32);
        variantTypes.push_back(f64);
        variantTypes.push_back(boolType);

        auto unionType = registry.getUnionType(variantTypes);
        REQUIRE(unionType->getAlignment() == 8);
        REQUIRE(unionType->getStaticSize() == 8);
        REQUIRE(unionType->hasStaticSize() == true);
        REQUIRE(unionType->isDynamicallySized() == false);
    }

    SECTION("UnionType equality and hashing") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        // Two unions with same variant types should be equal
        auto variantTypes1 = makeArenaVector<const Type*>(arena);
        variantTypes1.push_back(i32);
        variantTypes1.push_back(f64);

        auto variantTypes2 = makeArenaVector<const Type*>(arena);
        variantTypes2.push_back(i32);
        variantTypes2.push_back(f64);

        auto variantTypes3 = makeArenaVector<const Type*>(arena);
        variantTypes3.push_back(f64);
        variantTypes3.push_back(i32);  // Different order

        auto variantTypes4 = makeArenaVector<const Type*>(arena);
        variantTypes4.push_back(i32);
        variantTypes4.push_back(f64);
        variantTypes4.push_back(boolType);  // Different count

        auto union1 = registry.getUnionType(variantTypes1);
        auto union2 = registry.getUnionType(variantTypes2);
        auto union3 = registry.getUnionType(variantTypes3);
        auto union4 = registry.getUnionType(variantTypes4);

        REQUIRE(union1 == union2); // Same instance from cache
        REQUIRE(union1->equals(union2));
        REQUIRE_FALSE(union1->equals(union3)); // Different order
        REQUIRE_FALSE(union1->equals(union4)); // Different count

        // Hash should be consistent
        REQUIRE(union1->hash() == union2->hash());
    }
}

TEST_CASE("UnionType usage scenarios", "[types][union][usage]") {
    SECTION("Single variant union") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);

        auto variantTypes = makeArenaVector<const Type*>(arena);
        variantTypes.push_back(i32);

        auto unionType = registry.getUnionType(variantTypes);
        REQUIRE(unionType->toString() == "i32");  // Single variant, no pipes
        REQUIRE(unionType->getVariantCount() == 1);
        REQUIRE(unionType->getStaticSize() == 4);
    }

    SECTION("Multi-variant unions") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        auto variantTypes = makeArenaVector<const Type*>(arena);
        variantTypes.push_back(i32);
        variantTypes.push_back(f64);
        variantTypes.push_back(boolType);

        auto unionType = registry.getUnionType(variantTypes);
        REQUIRE(unionType->toString() == "i32 | f64 | bool");
        REQUIRE(unionType->getVariantCount() == 3);
    }

    SECTION("Union assignment compatibility") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        auto variantTypes = makeArenaVector<const Type*>(arena);
        variantTypes.push_back(i32);
        variantTypes.push_back(f64);
        variantTypes.push_back(boolType);

        auto unionType = registry.getUnionType(variantTypes);

        // Union should be assignable from any of its variant types
        REQUIRE(unionType->isAssignableFrom(i32) == true);
        REQUIRE(unionType->isAssignableFrom(f64) == true);
        REQUIRE(unionType->isAssignableFrom(boolType) == true);

        // But not from types that aren't variants
        auto charType = registry.charType();
        REQUIRE(unionType->isAssignableFrom(charType) == false);
    }

    SECTION("Compile-time properties") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        auto variantTypes = makeArenaVector<const Type*>(arena);
        variantTypes.push_back(i32);
        variantTypes.push_back(f64);
        variantTypes.push_back(boolType);

        auto unionType = registry.getUnionType(variantTypes);

        // Test variant access through direct methods
        REQUIRE(unionType->getVariantCount() == 3);
        REQUIRE(unionType->getVariantType(1) == f64);
        REQUIRE(unionType->getVariantType(10) == nullptr); // Out of bounds
    }
}

TEST_CASE("UnionType Phase 5 Completion Verification", "[types][union][phase5]") {
    SECTION("UnionType acceptance criteria verification") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        // ✓ UnionType class implementation
        auto variantTypes = makeArenaVector<const Type*>(arena);
        variantTypes.push_back(i32);
        variantTypes.push_back(f64);
        variantTypes.push_back(boolType);

        auto unionType = registry.getUnionType(variantTypes);
        REQUIRE(unionType != nullptr);
        REQUIRE(unionType->kind() == typUnion);

        // ✓ ArenaVector storage for variant types
        REQUIRE(unionType->getVariantCount() == 3);
        REQUIRE(unionType->getVariantType(0) == i32);
        REQUIRE(unionType->getVariantType(1) == f64);
        REQUIRE(unionType->getVariantType(2) == boolType);

        // ✓ Size and alignment calculations (max of all variants)
        REQUIRE(unionType->getAlignment() == 8); // max(4, 8, 1)
        REQUIRE(unionType->getStaticSize() == 8); // max(4, 8, 1)
        REQUIRE(unionType->hasStaticSize() == true);
        REQUIRE(unionType->isDynamicallySized() == false);

        // ✓ Union assignment semantics (assignable from any variant)
        REQUIRE(unionType->isAssignableFrom(i32) == true);
        REQUIRE(unionType->isAssignableFrom(f64) == true);
        REQUIRE(unionType->isAssignableFrom(boolType) == true);
        auto charType = registry.charType();
        REQUIRE(unionType->isAssignableFrom(charType) == false);

        // ✓ Direct variant access through type methods
        REQUIRE(unionType->getVariantCount() == 3);
        REQUIRE(unionType->getVariantType(1) == f64);

        // ✓ TypeRegistry caching
        auto unionType2 = registry.getUnionType(variantTypes);
        REQUIRE(unionType == unionType2); // Same instance from cache

        // ✓ Type equality and hashing
        auto otherVariants = makeArenaVector<const Type*>(arena);
        otherVariants.push_back(i32);
        otherVariants.push_back(f64);
        otherVariants.push_back(boolType);
        auto unionType3 = registry.getUnionType(otherVariants);
        REQUIRE(unionType->equals(unionType3));
        REQUIRE(unionType->hash() == unionType3->hash());

        // ✓ String representation
        REQUIRE(unionType->toString() == "i32 | f64 | bool");

        // ✓ Inheritance from CompositeType
        REQUIRE(unionType->isPrimitive() == false);
        REQUIRE(unionType->isComposite() == true);
        auto composite = unionType->as<CompositeType>();
        REQUIRE(composite != nullptr);

        INFO("UnionType implementation completed successfully!");
        INFO("Union semantics: assignable from any variant, max size/alignment");
        INFO("Ready for next composite type implementation");
    }
}

TEST_CASE("UnionType demonstration", "[types][union][demo]") {
    SECTION("UnionType provides flexible variant semantics") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();
        auto charType = registry.charType();

        // Create different union types to demonstrate flexibility

        // Numeric union: i32 | f64
        auto numericVariants = makeArenaVector<const Type*>(arena);
        numericVariants.push_back(i32);
        numericVariants.push_back(f64);
        auto numericUnion = registry.getUnionType(numericVariants);

        REQUIRE(numericUnion->toString() == "i32 | f64");
        REQUIRE(numericUnion->getStaticSize() == 8); // Size of f64
        REQUIRE(numericUnion->getAlignment() == 8); // Alignment of f64

        // Optional-like union: bool | i32
        auto optionalVariants = makeArenaVector<const Type*>(arena);
        optionalVariants.push_back(boolType);
        optionalVariants.push_back(i32);
        auto optionalUnion = registry.getUnionType(optionalVariants);

        REQUIRE(optionalUnion->toString() == "bool | i32");
        REQUIRE(optionalUnion->getStaticSize() == 4); // Size of i32
        REQUIRE(optionalUnion->getAlignment() == 4); // Alignment of i32

        // Multi-type union: i32 | f64 | bool | char
        auto multiVariants = makeArenaVector<const Type*>(arena);
        multiVariants.push_back(i32);
        multiVariants.push_back(f64);
        multiVariants.push_back(boolType);
        multiVariants.push_back(charType);
        auto multiUnion = registry.getUnionType(multiVariants);

        REQUIRE(multiUnion->toString() == "i32 | f64 | bool | char");
        REQUIRE(multiUnion->getStaticSize() == 8); // Size of largest (f64)

        // Test assignment semantics - unions accept any of their variants
        REQUIRE(numericUnion->isAssignableFrom(i32) == true);
        REQUIRE(numericUnion->isAssignableFrom(f64) == true);
        REQUIRE(numericUnion->isAssignableFrom(boolType) == false); // Not a variant

        REQUIRE(multiUnion->isAssignableFrom(i32) == true);
        REQUIRE(multiUnion->isAssignableFrom(f64) == true);
        REQUIRE(multiUnion->isAssignableFrom(boolType) == true);
        REQUIRE(multiUnion->isAssignableFrom(charType) == true);

        // Test variant access through type methods
        for (size_t i = 0; i < multiUnion->getVariantCount(); ++i) {
            auto variantType = multiUnion->getVariantType(i);
            REQUIRE(variantType != nullptr);
        }

        INFO("UnionType successfully demonstrates variant type semantics!");
        INFO("Unions provide type-safe sum types with runtime discrimination");
        INFO("Size is max of variants, assignable from any variant type");
    }
}

TEST_CASE("FunctionType basic functionality", "[types][function]") {
    SECTION("FunctionType should be included in TypeKind enum") {
        // Verify that function type kind exists
        REQUIRE(static_cast<int>(typFunction) != 0);
    }

    SECTION("FunctionType creation with TypeRegistry") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();
        auto voidType = registry.voidType();

        // Create parameter types vector
        auto paramTypes = makeArenaVector<const Type*>(arena);
        paramTypes.push_back(i32);
        paramTypes.push_back(f64);

        // Test FunctionType creation via TypeRegistry
        auto funcType = registry.getFunctionType(paramTypes, boolType);
        REQUIRE(funcType != nullptr);
        REQUIRE(funcType->getParameterCount() == 2);
        REQUIRE(funcType->getParameterType(0) == i32);
        REQUIRE(funcType->getParameterType(1) == f64);
        REQUIRE(funcType->getParameterType(2) == nullptr); // Out of bounds
        REQUIRE(funcType->getReturnType() == boolType);
    }
}

TEST_CASE("FunctionType implementation", "[types][function][implementation]") {
    SECTION("FunctionType inherits from CompositeType") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto boolType = registry.boolType();

        auto paramTypes = makeArenaVector<const Type*>(arena);
        paramTypes.push_back(i32);

        auto funcType = registry.getFunctionType(paramTypes, boolType);

        // Test inheritance and type classification
        REQUIRE(funcType->isPrimitive() == false);
        REQUIRE(funcType->isComposite() == true);
        REQUIRE(funcType->kind() == typFunction);

        // Test CompositeType casting
        auto composite = funcType->as<CompositeType>();
        REQUIRE(composite != nullptr);

        // Test callable classification
        REQUIRE(funcType->isCallable() == true);
    }

    SECTION("FunctionType size calculations") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        auto paramTypes = makeArenaVector<const Type*>(arena);
        paramTypes.push_back(i32);
        paramTypes.push_back(f64);

        auto funcType = registry.getFunctionType(paramTypes, boolType);

        // Function types should be pointer-sized
        REQUIRE(funcType->getStaticSize() == sizeof(void*));
        REQUIRE(funcType->getAlignment() == sizeof(void*));
        REQUIRE(funcType->hasStaticSize() == true);
        REQUIRE(funcType->isDynamicallySized() == false);
    }

    SECTION("FunctionType equality and hashing") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();
        auto voidType = registry.voidType();

        // Two functions with same signature should be equal
        auto paramTypes1 = makeArenaVector<const Type*>(arena);
        paramTypes1.push_back(i32);
        paramTypes1.push_back(f64);

        auto paramTypes2 = makeArenaVector<const Type*>(arena);
        paramTypes2.push_back(i32);
        paramTypes2.push_back(f64);

        auto paramTypes3 = makeArenaVector<const Type*>(arena);
        paramTypes3.push_back(f64);
        paramTypes3.push_back(i32);  // Different order

        auto paramTypes4 = makeArenaVector<const Type*>(arena);
        paramTypes4.push_back(i32);  // Different count

        auto func1 = registry.getFunctionType(paramTypes1, boolType);
        auto func2 = registry.getFunctionType(paramTypes2, boolType);
        auto func3 = registry.getFunctionType(paramTypes3, boolType);
        auto func4 = registry.getFunctionType(paramTypes4, boolType);
        auto func5 = registry.getFunctionType(paramTypes1, voidType); // Different return type

        REQUIRE(func1 == func2); // Same instance from cache
        REQUIRE(func1->equals(func2));
        REQUIRE_FALSE(func1->equals(func3)); // Different parameter order
        REQUIRE_FALSE(func1->equals(func4)); // Different parameter count
        REQUIRE_FALSE(func1->equals(func5)); // Different return type

        // Hash should be consistent
        REQUIRE(func1->hash() == func2->hash());
    }
}

TEST_CASE("FunctionType usage scenarios", "[types][function][usage]") {
    SECTION("No parameter functions") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto voidType = registry.voidType();

        // func() -> i32
        auto emptyParams = makeArenaVector<const Type*>(arena);
        auto funcType = registry.getFunctionType(emptyParams, i32);
        REQUIRE(funcType->toString() == "() -> i32");
        REQUIRE(funcType->getParameterCount() == 0);

        // func() -> void
        auto voidFunc = registry.getFunctionType(emptyParams, voidType);
        REQUIRE(voidFunc->toString() == "() -> void");
    }

    SECTION("Single parameter functions") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto boolType = registry.boolType();

        auto paramTypes = makeArenaVector<const Type*>(arena);
        paramTypes.push_back(i32);

        auto funcType = registry.getFunctionType(paramTypes, boolType);
        REQUIRE(funcType->toString() == "(i32) -> bool");
        REQUIRE(funcType->getParameterCount() == 1);
    }

    SECTION("Multi-parameter functions") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        auto paramTypes = makeArenaVector<const Type*>(arena);
        paramTypes.push_back(i32);
        paramTypes.push_back(f64);
        paramTypes.push_back(boolType);

        auto funcType = registry.getFunctionType(paramTypes, i32);
        REQUIRE(funcType->toString() == "(i32, f64, bool) -> i32");
        REQUIRE(funcType->getParameterCount() == 3);
    }

    SECTION("Higher-order functions") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto boolType = registry.boolType();

        // Create (i32) -> bool function type
        auto innerParams = makeArenaVector<const Type*>(arena);
        innerParams.push_back(i32);
        auto innerFunc = registry.getFunctionType(innerParams, boolType);

        // Create higher-order function: ((i32) -> bool) -> i32
        auto outerParams = makeArenaVector<const Type*>(arena);
        outerParams.push_back(innerFunc);
        auto outerFunc = registry.getFunctionType(outerParams, i32);

        REQUIRE(outerFunc->toString() == "((i32) -> bool) -> i32");
        REQUIRE(outerFunc->getParameterCount() == 1);
        REQUIRE(outerFunc->getParameterType(0) == innerFunc);
        REQUIRE(outerFunc->isCallable() == true);
    }

    SECTION("Function assignment compatibility") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        auto paramTypes1 = makeArenaVector<const Type*>(arena);
        paramTypes1.push_back(i32);
        auto func1 = registry.getFunctionType(paramTypes1, boolType);

        auto paramTypes2 = makeArenaVector<const Type*>(arena);
        paramTypes2.push_back(i32);
        auto func2 = registry.getFunctionType(paramTypes2, boolType);

        auto paramTypes3 = makeArenaVector<const Type*>(arena);
        paramTypes3.push_back(f64);
        auto func3 = registry.getFunctionType(paramTypes3, boolType);

        // Functions with identical signatures should be assignable
        REQUIRE(func1->isAssignableFrom(func2) == true);

        // Functions with different signatures should not be assignable
        REQUIRE(func1->isAssignableFrom(func3) == false);
    }

    SECTION("Compile-time properties") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        auto paramTypes = makeArenaVector<const Type*>(arena);
        paramTypes.push_back(i32);
        paramTypes.push_back(f64);

        auto funcType = registry.getFunctionType(paramTypes, boolType);

        // Test direct parameter access through type methods
        REQUIRE(funcType->getParameterCount() == 2);
        REQUIRE(funcType->getParameterType(0) == i32);
        REQUIRE(funcType->getReturnType() == boolType);
        REQUIRE(funcType->getParameterType(10) == nullptr); // Out of bounds
    }
}

TEST_CASE("FunctionType Completion Verification", "[types][function][completion]") {
    SECTION("FunctionType acceptance criteria verification") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();
        auto voidType = registry.voidType();

        // ✓ FunctionType class implementation
        auto paramTypes = makeArenaVector<const Type*>(arena);
        paramTypes.push_back(i32);
        paramTypes.push_back(f64);

        auto funcType = registry.getFunctionType(paramTypes, boolType);
        REQUIRE(funcType != nullptr);
        REQUIRE(funcType->kind() == typFunction);

        // ✓ ArenaVector storage for parameter types + return type
        REQUIRE(funcType->getParameterCount() == 2);
        REQUIRE(funcType->getParameterType(0) == i32);
        REQUIRE(funcType->getParameterType(1) == f64);
        REQUIRE(funcType->getReturnType() == boolType);

        // ✓ Function pointer semantics (pointer-sized)
        REQUIRE(funcType->getStaticSize() == sizeof(void*));
        REQUIRE(funcType->getAlignment() == sizeof(void*));
        REQUIRE(funcType->hasStaticSize() == true);
        REQUIRE(funcType->isDynamicallySized() == false);

        // ✓ Callable type classification
        REQUIRE(funcType->isCallable() == true);
        REQUIRE(funcType->isPrimitive() == false);
        REQUIRE(funcType->isComposite() == true);

        // ✓ Function signature semantics (identical signatures assignable)
        auto sameSigParams = makeArenaVector<const Type*>(arena);
        sameSigParams.push_back(i32);
        sameSigParams.push_back(f64);
        auto sameSigFunc = registry.getFunctionType(sameSigParams, boolType);
        REQUIRE(funcType->isAssignableFrom(sameSigFunc) == true);

        // Different signature should not be assignable
        auto diffSigParams = makeArenaVector<const Type*>(arena);
        diffSigParams.push_back(f64);
        diffSigParams.push_back(i32);
        auto diffSigFunc = registry.getFunctionType(diffSigParams, boolType);
        REQUIRE(funcType->isAssignableFrom(diffSigFunc) == false);

        // ✓ Direct parameter and return type access
        REQUIRE(funcType->getParameterCount() == 2);
        REQUIRE(funcType->getParameterType(1) == f64);
        REQUIRE(funcType->getReturnType() == boolType);

        // ✓ TypeRegistry caching
        auto funcType2 = registry.getFunctionType(paramTypes, boolType);
        REQUIRE(funcType == funcType2); // Same instance from cache

        // ✓ Type equality and hashing
        auto otherParams = makeArenaVector<const Type*>(arena);
        otherParams.push_back(i32);
        otherParams.push_back(f64);
        auto funcType3 = registry.getFunctionType(otherParams, boolType);
        REQUIRE(funcType->equals(funcType3));
        REQUIRE(funcType->hash() == funcType3->hash());

        // ✓ String representation
        REQUIRE(funcType->toString() == "(i32, f64) -> bool");

        // ✓ No parameter and void return functions
        auto emptyParams = makeArenaVector<const Type*>(arena);
        auto voidFunc = registry.getFunctionType(emptyParams, voidType);
        REQUIRE(voidFunc->toString() == "() -> void");
        REQUIRE(voidFunc->getParameterCount() == 0);

        // ✓ Inheritance from CompositeType
        auto composite = funcType->as<CompositeType>();
        REQUIRE(composite != nullptr);

        INFO("FunctionType implementation completed successfully!");
        INFO("Function semantics: callable, pointer-sized, signature-based assignment");
        INFO("Supports higher-order functions and compile-time introspection");
    }
}

TEST_CASE("FunctionType demonstration", "[types][function][demo]") {
    SECTION("FunctionType provides powerful function signature semantics") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();
        auto voidType = registry.voidType();

        // Basic function types

        // Simple function: (i32) -> bool
        auto simpleParams = makeArenaVector<const Type*>(arena);
        simpleParams.push_back(i32);
        auto simpleFunc = registry.getFunctionType(simpleParams, boolType);

        REQUIRE(simpleFunc->toString() == "(i32) -> bool");
        REQUIRE(simpleFunc->isCallable() == true);
        REQUIRE(simpleFunc->getParameterCount() == 1);
        REQUIRE(simpleFunc->getReturnType() == boolType);

        // No-parameter function: () -> i32
        auto noParams = makeArenaVector<const Type*>(arena);
        auto noParamFunc = registry.getFunctionType(noParams, i32);

        REQUIRE(noParamFunc->toString() == "() -> i32");
        REQUIRE(noParamFunc->getParameterCount() == 0);

        // Multi-parameter function: (i32, f64, bool) -> void
        auto multiParams = makeArenaVector<const Type*>(arena);
        multiParams.push_back(i32);
        multiParams.push_back(f64);
        multiParams.push_back(boolType);
        auto multiFunc = registry.getFunctionType(multiParams, voidType);

        REQUIRE(multiFunc->toString() == "(i32, f64, bool) -> void");
        REQUIRE(multiFunc->getParameterCount() == 3);

        // Higher-order functions - functions that take/return functions

        // Predicate type: (i32) -> bool
        auto predicateParams = makeArenaVector<const Type*>(arena);
        predicateParams.push_back(i32);
        auto predicateType = registry.getFunctionType(predicateParams, boolType);

        // Filter function: ((i32) -> bool) -> bool
        auto filterParams = makeArenaVector<const Type*>(arena);
        filterParams.push_back(predicateType);
        auto filterType = registry.getFunctionType(filterParams, boolType);

        REQUIRE(filterType->toString() == "((i32) -> bool) -> bool");
        REQUIRE(filterType->getParameterType(0) == predicateType);
        REQUIRE(filterType->getParameterType(0)->isCallable() == true);

        // Function factory: () -> ((i32) -> bool)
        auto factoryParams = makeArenaVector<const Type*>(arena);
        auto factoryType = registry.getFunctionType(factoryParams, predicateType);

        REQUIRE(factoryType->toString() == "() -> ((i32) -> bool)");
        REQUIRE(factoryType->getReturnType() == predicateType);
        REQUIRE(factoryType->getReturnType()->isCallable() == true);

        // Curried function: (i32) -> ((f64) -> bool)
        auto innerCurriedParams = makeArenaVector<const Type*>(arena);
        innerCurriedParams.push_back(f64);
        auto innerCurriedType = registry.getFunctionType(innerCurriedParams, boolType);

        auto outerCurriedParams = makeArenaVector<const Type*>(arena);
        outerCurriedParams.push_back(i32);
        auto curriedType = registry.getFunctionType(outerCurriedParams, innerCurriedType);

        REQUIRE(curriedType->toString() == "(i32) -> ((f64) -> bool)");

        // Test direct type introspection of complex function types

        // Test parameter access for higher-order functions
        REQUIRE(filterType->getParameterType(0) == predicateType);

        // Test return type access
        REQUIRE(factoryType->getReturnType() == predicateType);

        // Function assignment semantics - only identical signatures are assignable
        auto identicalPredicateParams = makeArenaVector<const Type*>(arena);
        identicalPredicateParams.push_back(i32);
        auto identicalPredicate = registry.getFunctionType(identicalPredicateParams, boolType);

        REQUIRE(predicateType->isAssignableFrom(identicalPredicate) == true);

        // Different signatures are not assignable
        auto differentParams = makeArenaVector<const Type*>(arena);
        differentParams.push_back(f64); // Different parameter type
        auto differentFunc = registry.getFunctionType(differentParams, boolType);

        REQUIRE(predicateType->isAssignableFrom(differentFunc) == false);

        // All function types are pointer-sized
        REQUIRE(simpleFunc->getStaticSize() == sizeof(void*));
        REQUIRE(filterType->getStaticSize() == sizeof(void*));
        REQUIRE(factoryType->getStaticSize() == sizeof(void*));
        REQUIRE(curriedType->getStaticSize() == sizeof(void*));

        INFO("FunctionType successfully demonstrates function signature semantics!");
        INFO("Supports higher-order functions, currying, and compile-time introspection");
        INFO("Function types are pointer-sized and enable functional programming patterns");
    }
}

TEST_CASE("Type system API verification", "[types][api]") {
    SECTION("All composite types provide clean direct access APIs") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        // ArrayType API: getArraySize, isDynamicArray, getElementType
        auto arrayType = registry.getArrayType(i32, 10);
        REQUIRE(arrayType->getArraySize() == 10);
        REQUIRE(arrayType->isDynamicArray() == false);
        REQUIRE(arrayType->getElementType() == i32);

        // TupleType API: getElementCount, getElementType
        auto tupleElements = makeArenaVector<const Type*>(arena);
        tupleElements.push_back(i32);
        tupleElements.push_back(f64);
        auto tupleType = registry.getTupleType(tupleElements);
        REQUIRE(tupleType->getElementCount() == 2);
        REQUIRE(tupleType->getElementType(0) == i32);
        REQUIRE(tupleType->getElementType(1) == f64);

        // UnionType API: getVariantCount, getVariantType
        auto unionVariants = makeArenaVector<const Type*>(arena);
        unionVariants.push_back(i32);
        unionVariants.push_back(boolType);
        auto unionType = registry.getUnionType(unionVariants);
        REQUIRE(unionType->getVariantCount() == 2);
        REQUIRE(unionType->getVariantType(0) == i32);
        REQUIRE(unionType->getVariantType(1) == boolType);

        // FunctionType API: getParameterCount, getParameterType, getReturnType
        auto funcParams = makeArenaVector<const Type*>(arena);
        funcParams.push_back(i32);
        funcParams.push_back(f64);
        auto funcType = registry.getFunctionType(funcParams, boolType);
        REQUIRE(funcType->getParameterCount() == 2);
        REQUIRE(funcType->getParameterType(0) == i32);
        REQUIRE(funcType->getParameterType(1) == f64);
        REQUIRE(funcType->getReturnType() == boolType);

        INFO("✅ All composite types provide clean direct access APIs:");
        INFO("  Types focus on core responsibilities: structure, size, alignment, compatibility");
        INFO("  Compile-time features handled by semantic analysis, not type system");
    }
}

TEST_CASE("Type System Design Improvement Summary", "[types][design]") {
    SECTION("Cleaner separation of concerns after removing compile-time properties") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        // ✅ Types now focus on their core responsibilities

        // ArrayType: structure, element access, size calculations
        auto arrayType = registry.getArrayType(i32, 10);
        REQUIRE(arrayType->getElementType() == i32);
        REQUIRE(arrayType->getArraySize() == 10);
        REQUIRE(arrayType->getStaticSize() == 40); // Core responsibility: size
        REQUIRE(arrayType->isFixedArray() == true); // Core responsibility: structure

        // TupleType: heterogeneous element management
        auto tupleElements = makeArenaVector<const Type*>(arena);
        tupleElements.push_back(i32);
        tupleElements.push_back(f64);
        auto tupleType = registry.getTupleType(tupleElements);
        REQUIRE(tupleType->getElementCount() == 2); // Core responsibility: structure
        REQUIRE(tupleType->getElementType(0) == i32); // Core responsibility: access
        REQUIRE(tupleType->getStaticSize() == 16); // Core responsibility: size

        // UnionType: variant management and assignment semantics
        auto unionVariants = makeArenaVector<const Type*>(arena);
        unionVariants.push_back(i32);
        unionVariants.push_back(boolType);
        auto unionType = registry.getUnionType(unionVariants);
        REQUIRE(unionType->getVariantCount() == 2); // Core responsibility: structure
        REQUIRE(unionType->isAssignableFrom(i32) == true); // Core responsibility: compatibility
        REQUIRE(unionType->getStaticSize() == 4); // Core responsibility: size (max of variants)

        // FunctionType: signature management and callable semantics
        auto funcParams = makeArenaVector<const Type*>(arena);
        funcParams.push_back(i32);
        auto funcType = registry.getFunctionType(funcParams, boolType);
        REQUIRE(funcType->getParameterCount() == 1); // Core responsibility: structure
        REQUIRE(funcType->getReturnType() == boolType); // Core responsibility: access
        REQUIRE(funcType->isCallable() == true); // Core responsibility: classification
        REQUIRE(funcType->getStaticSize() == sizeof(void*)); // Core responsibility: size
    }
}

TEST_CASE("FunctionType argument compatibility", "[types][function][calls]") {
    SECTION("canBeCalledWith checks argument type compatibility") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i8 = registry.integerType(IntegerKind::I8);
        auto i32 = registry.integerType(IntegerKind::I32);
        auto i64 = registry.integerType(IntegerKind::I64);
        auto f32 = registry.floatType(FloatKind::F32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        // Create function: (i32, f64) -> bool
        auto paramTypes = makeArenaVector<const Type*>(arena);
        paramTypes.push_back(i32);
        paramTypes.push_back(f64);
        auto funcType = registry.getFunctionType(paramTypes, boolType);

        // Test exact match - should work
        auto exactArgs = makeArenaVector<const Type*>(arena);
        exactArgs.push_back(i32);
        exactArgs.push_back(f64);
        REQUIRE(funcType->canBeCalledWith(exactArgs) == true);

        // Test implicit conversions - should work
        auto implicitArgs = makeArenaVector<const Type*>(arena);
        implicitArgs.push_back(i8);   // i8 -> i32 (widening)
        implicitArgs.push_back(f32);  // f32 -> f64 (widening)
        REQUIRE(funcType->canBeCalledWith(implicitArgs) == true);

        // Test wrong argument count - should fail
        auto wrongCountArgs = makeArenaVector<const Type*>(arena);
        wrongCountArgs.push_back(i32);
        // Missing second argument
        REQUIRE(funcType->canBeCalledWith(wrongCountArgs) == false);

        // Test too many arguments - should fail
        auto tooManyArgs = makeArenaVector<const Type*>(arena);
        tooManyArgs.push_back(i32);
        tooManyArgs.push_back(f64);
        tooManyArgs.push_back(boolType);
        REQUIRE(funcType->canBeCalledWith(tooManyArgs) == false);

        // Test incompatible types - should fail
        auto incompatibleArgs = makeArenaVector<const Type*>(arena);
        incompatibleArgs.push_back(boolType);  // bool cannot convert to i32
        incompatibleArgs.push_back(f64);
        REQUIRE(funcType->canBeCalledWith(incompatibleArgs) == false);
    }

    SECTION("getConversionDistance calculates conversion costs") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i8 = registry.integerType(IntegerKind::I8);
        auto i32 = registry.integerType(IntegerKind::I32);
        auto i64 = registry.integerType(IntegerKind::I64);
        auto f32 = registry.floatType(FloatKind::F32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        // Create function: (i32, f64) -> bool
        auto paramTypes = makeArenaVector<const Type*>(arena);
        paramTypes.push_back(i32);
        paramTypes.push_back(f64);
        auto funcType = registry.getFunctionType(paramTypes, boolType);

        // Test exact match - distance should be 0
        auto exactArgs = makeArenaVector<const Type*>(arena);
        exactArgs.push_back(i32);
        exactArgs.push_back(f64);
        REQUIRE(funcType->getConversionDistance(exactArgs) == 0);

        // Test widening conversions - should have low distance
        auto wideningArgs = makeArenaVector<const Type*>(arena);
        wideningArgs.push_back(i8);   // i8 -> i32 (widening)
        wideningArgs.push_back(f32);  // f32 -> f64 (widening)
        int wideningDistance = funcType->getConversionDistance(wideningArgs);
        REQUIRE(wideningDistance > 0);
        REQUIRE(wideningDistance <= 2); // Should be cheap

        // Test narrowing conversions - should have higher distance
        auto narrowingArgs = makeArenaVector<const Type*>(arena);
        narrowingArgs.push_back(i64);  // i64 -> i32 (narrowing)
        narrowingArgs.push_back(f64);  // exact match for second param
        int narrowingDistance = funcType->getConversionDistance(narrowingArgs);
        REQUIRE(narrowingDistance > wideningDistance); // More expensive than widening

        // Test impossible conversion - should return -1
        auto impossibleArgs = makeArenaVector<const Type*>(arena);
        impossibleArgs.push_back(boolType);  // bool cannot convert to i32
        impossibleArgs.push_back(f64);
        REQUIRE(funcType->getConversionDistance(impossibleArgs) == -1);

        // Test wrong argument count - should return -1
        auto wrongCountArgs = makeArenaVector<const Type*>(arena);
        wrongCountArgs.push_back(i32);
        // Missing second argument
        REQUIRE(funcType->getConversionDistance(wrongCountArgs) == -1);
    }

    SECTION("Function overload resolution example") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i8 = registry.integerType(IntegerKind::I8);
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        // Create overloaded functions
        // func1: (i32) -> bool
        auto func1Params = makeArenaVector<const Type*>(arena);
        func1Params.push_back(i32);
        auto func1 = registry.getFunctionType(func1Params, boolType);

        // func2: (f64) -> bool
        auto func2Params = makeArenaVector<const Type*>(arena);
        func2Params.push_back(f64);
        auto func2 = registry.getFunctionType(func2Params, boolType);

        // Call with i8 argument
        auto callArgs = makeArenaVector<const Type*>(arena);
        callArgs.push_back(i8);

        // Both functions can be called with i8
        REQUIRE(func1->canBeCalledWith(callArgs) == true);
        REQUIRE(func2->canBeCalledWith(callArgs) == true);

        // But func1 should be preferred (better conversion distance)
        int func1Distance = func1->getConversionDistance(callArgs);
        int func2Distance = func2->getConversionDistance(callArgs);

        REQUIRE(func1Distance >= 0);
        REQUIRE(func2Distance >= 0);
        REQUIRE(func1Distance < func2Distance); // i8->i32 is better than i8->f64

        INFO("Function overload resolution works: func1 (i8->i32) preferred over func2 (i8->f64)");
    }
}

TEST_CASE("Function call compatibility API summary", "[types][function][api]") {
    SECTION("Comprehensive demonstration of function argument checking") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i8 = registry.integerType(IntegerKind::I8);
        auto i32 = registry.integerType(IntegerKind::I32);
        auto i64 = registry.integerType(IntegerKind::I64);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();

        // Example function: processData(i32, f64) -> bool
        auto funcParams = makeArenaVector<const Type*>(arena);
        funcParams.push_back(i32);
        funcParams.push_back(f64);
        auto processFunc = registry.getFunctionType(funcParams, boolType);

        // ✅ Perfect API for function call validation

        // Test exact match (perfect call)
        auto exactArgs = makeArenaVector<const Type*>(arena);
        exactArgs.push_back(i32);
        exactArgs.push_back(f64);
        REQUIRE(processFunc->canBeCalledWith(exactArgs) == true);
        REQUIRE(processFunc->getConversionDistance(exactArgs) == 0);

        // Test widening conversions (good call)
        auto wideningArgs = makeArenaVector<const Type*>(arena);
        wideningArgs.push_back(i8);   // i8 -> i32 (widens)
        wideningArgs.push_back(f64);  // exact match
        REQUIRE(processFunc->canBeCalledWith(wideningArgs) == true);
        int wideningDist = processFunc->getConversionDistance(wideningArgs);
        REQUIRE(wideningDist > 0);
        REQUIRE(wideningDist < 3); // Should be cheap

        // Test narrowing conversions (acceptable but expensive call)
        auto narrowingArgs = makeArenaVector<const Type*>(arena);
        narrowingArgs.push_back(i64);  // i64 -> i32 (narrows)
        narrowingArgs.push_back(f64);  // exact match
        REQUIRE(processFunc->canBeCalledWith(narrowingArgs) == true);
        int narrowingDist = processFunc->getConversionDistance(narrowingArgs);
        REQUIRE(narrowingDist > wideningDist); // More expensive than widening

        // Test impossible calls
        auto impossibleArgs = makeArenaVector<const Type*>(arena);
        impossibleArgs.push_back(boolType);  // bool can't convert to i32
        impossibleArgs.push_back(f64);
        REQUIRE(processFunc->canBeCalledWith(impossibleArgs) == false);
        REQUIRE(processFunc->getConversionDistance(impossibleArgs) == -1);

        // ✅ Perfect for overload resolution

        // Create competing functions
        auto func1Params = makeArenaVector<const Type*>(arena);
        func1Params.push_back(i32);
        auto func1 = registry.getFunctionType(func1Params, boolType); // (i32) -> bool

        auto func2Params = makeArenaVector<const Type*>(arena);
        func2Params.push_back(f64);
        auto func2 = registry.getFunctionType(func2Params, boolType); // (f64) -> bool

        // Call with i8 - both can handle it
        auto ambiguousCall = makeArenaVector<const Type*>(arena);
        ambiguousCall.push_back(i8);

        REQUIRE(func1->canBeCalledWith(ambiguousCall) == true);
        REQUIRE(func2->canBeCalledWith(ambiguousCall) == true);

        // But func1 is better (closer conversion)
        int func1Dist = func1->getConversionDistance(ambiguousCall);
        int func2Dist = func2->getConversionDistance(ambiguousCall);
        REQUIRE(func1Dist < func2Dist); // i8->i32 better than i8->f64
    }
}
