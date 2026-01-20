#include "cxy/types.hpp"
#include "cxy/types/registry.hpp"
#include "cxy/arena_allocator.hpp"
#include "cxy/ast/types.hpp"
#include "cxy/token.hpp"
#include "cxy/strings.hpp"
#include "catch2.hpp"

using namespace cxy;
using namespace cxy::ast;

namespace {

// Test fixture for struct type tests
class StructTypeTestFixture {
public:
    StructTypeTestFixture() : interner(arena) {
        // Get basic primitive types
        i32Type = registry.getIntegerType(IntegerKind::I32);
        i64Type = registry.getIntegerType(IntegerKind::I64);
        f64Type = registry.getFloatType(FloatKind::F64);
        boolType = registry.getBoolType();
        charType = registry.getCharType();
    }

    InternedString intern(const std::string& str) {
        return interner.intern(str);
    }
    
    // Helper for empty methods array
    ArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>> emptyMethods() {
        return ArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>{
            ArenaSTLAllocator<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(arena)
        };
    }

    TypeRegistry& registry = TypeRegistry::instance();
    ArenaAllocator arena;
    StringInterner interner;

    // Common types
    const IntegerType* i32Type;
    const IntegerType* i64Type;
    const FloatType* f64Type;
    const BoolType* boolType;
    const CharType* charType;
};

} // namespace

TEST_CASE_METHOD(StructTypeTestFixture, "StructType - Basic Creation and Properties", "[types][struct]") {
    SECTION("Create simple struct with primitive fields") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("x"), i32Type));
        fields.emplace_back(std::make_pair(intern("y"), i32Type));

        auto* pointStruct = registry.getStructType(intern("Point"), std::move(fields), emptyMethods(), flgNone, nullptr);

        REQUIRE(pointStruct != nullptr);
        REQUIRE(pointStruct->kind() == typStruct);
        REQUIRE(pointStruct->getName().view() == "Point");
        REQUIRE(!pointStruct->isAnonymous());
        REQUIRE(!pointStruct->isPacked());
        REQUIRE(pointStruct->getFieldCount() == 2);
        REQUIRE(pointStruct->isComposite());
        REQUIRE(!pointStruct->isPrimitive());
        REQUIRE(pointStruct->hasStaticSize());
        REQUIRE(!pointStruct->isDynamicallySized());
    }

    SECTION("Create anonymous struct") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("value"), i32Type));

        auto* anonStruct = registry.getStructType(intern(""), std::move(fields), emptyMethods(), flgNone, nullptr);

        REQUIRE(anonStruct != nullptr);
        REQUIRE(anonStruct->getName().empty());
        REQUIRE(anonStruct->isAnonymous());
        REQUIRE(anonStruct->getFieldCount() == 1);
    }

    SECTION("Create empty struct") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};

        auto* emptyStruct = registry.getStructType(intern("Empty"), std::move(fields), emptyMethods(), flgNone, nullptr);

        REQUIRE(emptyStruct != nullptr);
        REQUIRE(emptyStruct->getFieldCount() == 0);
        REQUIRE(emptyStruct->getStaticSize() == 1); // C++ style: size 1 for empty structs
        REQUIRE(emptyStruct->getAlignment() == 1);
    }

    SECTION("Create packed struct") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("a"), charType));
        fields.emplace_back(std::make_pair(intern("b"), i32Type));

        auto* packedStruct = registry.getStructType(intern("Packed"), std::move(fields), emptyMethods(), flgPacked, nullptr);

        REQUIRE(packedStruct != nullptr);
        REQUIRE(packedStruct->isPacked());
        REQUIRE(packedStruct->hasFlag(flgPacked));
        REQUIRE(packedStruct->getStaticSize() == 8); // 1 + 8, no padding
    }
}

TEST_CASE_METHOD(StructTypeTestFixture, "StructType - Field Access", "[types][struct]") {
    ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
    fields.emplace_back(std::make_pair(intern("name"), charType));
    fields.emplace_back(std::make_pair(intern("age"), i32Type));
    fields.emplace_back(std::make_pair(intern("height"), f64Type));
    fields.emplace_back(std::make_pair(intern("active"), boolType));

    auto* personStruct = registry.getStructType(intern("Person"), std::move(fields), emptyMethods(), flgNone, nullptr);

    SECTION("Field lookup by name") {
        REQUIRE(personStruct->hasField(intern("name")));
        REQUIRE(personStruct->hasField(intern("age")));
        REQUIRE(personStruct->hasField(intern("height")));
        REQUIRE(personStruct->hasField(intern("active")));
        REQUIRE(!personStruct->hasField(intern("weight")));
        REQUIRE(!personStruct->hasField(intern("")));

        REQUIRE(personStruct->getFieldType(intern("name")) == charType);
        REQUIRE(personStruct->getFieldType(intern("age")) == i32Type);
        REQUIRE(personStruct->getFieldType(intern("height")) == f64Type);
        REQUIRE(personStruct->getFieldType(intern("active")) == boolType);
        REQUIRE(personStruct->getFieldType(intern("nonexistent")) == nullptr);
    }

    SECTION("Field lookup by index") {
        REQUIRE(personStruct->getFieldIndex(intern("name")) == 0);
        REQUIRE(personStruct->getFieldIndex(intern("age")) == 1);
        REQUIRE(personStruct->getFieldIndex(intern("height")) == 2);
        REQUIRE(personStruct->getFieldIndex(intern("active")) == 3);
        REQUIRE(personStruct->getFieldIndex(intern("nonexistent")) == SIZE_MAX);
    }

    SECTION("Field access through getFields") {
        const auto& fieldList = personStruct->getFields();
        REQUIRE(fieldList.size() == 4);

        REQUIRE(fieldList[0].name == intern("name"));
        REQUIRE(fieldList[0].type == charType);

        REQUIRE(fieldList[1].name == intern("age"));
        REQUIRE(fieldList[1].type == i32Type);

        REQUIRE(fieldList[2].name == intern("height"));
        REQUIRE(fieldList[2].type == f64Type);

        REQUIRE(fieldList[3].name == intern("active"));
        REQUIRE(fieldList[3].type == boolType);
    }
}

TEST_CASE_METHOD(StructTypeTestFixture, "StructType - Layout Calculations", "[types][struct]") {
    SECTION("Natural alignment layout") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("flag"), boolType));    // 1 byte
        fields.emplace_back(std::make_pair(intern("value"), i64Type));    // 8 bytes

        auto* naturalStruct = registry.getStructType(intern("Natural"), std::move(fields), emptyMethods(), flgNone, nullptr);

        // Natural alignment: flag(1) + padding(7) + value(8) = 16 bytes
        REQUIRE(naturalStruct->getStaticSize() == 16);
        REQUIRE(naturalStruct->getAlignment() == 8); // Aligned to largest field
        REQUIRE(!naturalStruct->isPacked());
    }

    SECTION("Packed layout") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("flag"), boolType));    // 1 byte
        fields.emplace_back(std::make_pair(intern("value"), i64Type));    // 8 bytes

        auto* packedStruct = registry.getStructType(intern("Packed"), std::move(fields), emptyMethods(), flgPacked, nullptr);

        // Packed: flag(1) + value(8) = 9 bytes, no padding
        REQUIRE(packedStruct->getStaticSize() == 9);
        REQUIRE(packedStruct->getAlignment() == 1); // No alignment requirements
        REQUIRE(packedStruct->isPacked());
    }

    SECTION("Complex field layout") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("a"), charType));  // 1 byte
        fields.emplace_back(std::make_pair(intern("b"), i32Type));   // 4 bytes
        fields.emplace_back(std::make_pair(intern("c"), i64Type));   // 8 bytes

        auto* complexStruct = registry.getStructType(intern("Complex"), std::move(fields), emptyMethods(), flgNone, nullptr);

        // Natural: a(1) + padding(3) + b(4) + c(8) = 16 bytes
        REQUIRE(complexStruct->getStaticSize() == 16);
        REQUIRE(complexStruct->getAlignment() == 8);
    }

    SECTION("Field offset calculations") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("a"), charType));  // offset 0
        fields.emplace_back(std::make_pair(intern("b"), i32Type));   // offset 4 (aligned)
        fields.emplace_back(std::make_pair(intern("c"), i64Type));   // offset 8 (aligned)

        auto* offsetStruct = registry.getStructType(intern("Offsets"), std::move(fields), emptyMethods(), flgNone, nullptr);

        REQUIRE(offsetStruct->getFieldOffset(0) == 0);  // a at offset 0
        REQUIRE(offsetStruct->getFieldOffset(1) == 4);  // b at offset 4
        REQUIRE(offsetStruct->getFieldOffset(2) == 8);  // c at offset 8
        REQUIRE(offsetStruct->getFieldOffset(SIZE_MAX) == SIZE_MAX);

        REQUIRE(offsetStruct->getFieldOffset(intern("a")) == 0);
        REQUIRE(offsetStruct->getFieldOffset(intern("b")) == 4);
        REQUIRE(offsetStruct->getFieldOffset(intern("c")) == 8);
        REQUIRE(offsetStruct->getFieldOffset(intern("nonexistent")) == SIZE_MAX);
    }
}

TEST_CASE_METHOD(StructTypeTestFixture, "StructType - Type Identity and Equality", "[types][struct]") {
    SECTION("Same struct should return same instance (registry caching)") {
        ArenaVector<std::pair<InternedString, const Type*>> fields1{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields1.emplace_back(std::make_pair(intern("x"), i32Type));
        fields1.emplace_back(std::make_pair(intern("y"), i32Type));

        ArenaVector<std::pair<InternedString, const Type*>> fields2{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields2.emplace_back(std::make_pair(intern("x"), i32Type));
        fields2.emplace_back(std::make_pair(intern("y"), i32Type));

        auto* struct1 = registry.getStructType(intern("Point"), std::move(fields1), emptyMethods(), flgNone, nullptr);
        auto* struct2 = registry.getStructType(intern("Point"), std::move(fields2), emptyMethods(), flgNone, nullptr);

        REQUIRE(struct1 == struct2); // Same instance from registry
        REQUIRE(struct1->equals(struct2));
    }

    SECTION("Different field names create different types") {
        ArenaVector<std::pair<InternedString, const Type*>> fields1{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields1.emplace_back(std::make_pair(intern("x"), i32Type));

        ArenaVector<std::pair<InternedString, const Type*>> fields2{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields2.emplace_back(std::make_pair(intern("y"), i32Type));

        auto* struct1 = registry.getStructType(intern("Test1"), std::move(fields1), emptyMethods(), flgNone, nullptr);
        auto* struct2 = registry.getStructType(intern("Test1"), std::move(fields2), emptyMethods(), flgNone, nullptr);

        REQUIRE(struct1 != struct2);
        REQUIRE(!struct1->equals(struct2));
    }

    SECTION("Different field types create different types") {
        ArenaVector<std::pair<InternedString, const Type*>> fields1{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields1.emplace_back(std::make_pair(intern("value"), i32Type));

        ArenaVector<std::pair<InternedString, const Type*>> fields2{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields2.emplace_back(std::make_pair(intern("value"), i64Type));

        auto* struct1 = registry.getStructType(intern("Test1"), std::move(fields1), emptyMethods(), flgNone, nullptr);
        auto* struct2 = registry.getStructType(intern("Test1"), std::move(fields2), emptyMethods(), flgNone, nullptr);

        REQUIRE(!struct1->equals(struct2));
    }

    SECTION("Flag differences affect identity") {
        ArenaVector<std::pair<InternedString, const Type*>> fields1{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields1.emplace_back(std::make_pair(intern("x"), i32Type));

        ArenaVector<std::pair<InternedString, const Type*>> fields2{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields2.emplace_back(std::make_pair(intern("x"), i32Type));

        auto* struct1 = registry.getStructType(intern("Test"), std::move(fields1), emptyMethods(), flgNone, nullptr);
        auto* struct2 = registry.getStructType(intern("Test"), std::move(fields2), emptyMethods(), flgPacked, nullptr);

        REQUIRE(!struct1->equals(struct2));
    }

    SECTION("Different flags create different types") {
        ArenaVector<std::pair<InternedString, const Type*>> fields1{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields1.emplace_back(std::make_pair(intern("value"), i32Type));

        ArenaVector<std::pair<InternedString, const Type*>> fields2{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields2.emplace_back(std::make_pair(intern("value"), i32Type));

        auto* naturalStruct = registry.getStructType(intern("Test"), std::move(fields1), emptyMethods(), flgNone, nullptr);
        auto* packedStruct = registry.getStructType(intern("TestPacked"), std::move(fields2), emptyMethods(), flgPacked, nullptr);

        REQUIRE(!naturalStruct->equals(packedStruct));
        REQUIRE(naturalStruct->getStaticSize() == packedStruct->getStaticSize()); // Same for single i32
        REQUIRE(naturalStruct->getAlignment() == 4); // Natural alignment = field alignment
        REQUIRE(packedStruct->getAlignment() == 1);   // Packed alignment = 1
    }
}

TEST_CASE_METHOD(StructTypeTestFixture, "StructType - String Representation", "[types][struct]") {
    SECTION("Named struct toString") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("x"), i32Type));
        fields.emplace_back(std::make_pair(intern("y"), i32Type));

        auto* pointStruct = registry.getStructType(intern("Point"), std::move(fields), emptyMethods(), flgNone, nullptr);
        auto str = pointStruct->toString();

        REQUIRE(str.find("Point") != std::string::npos);
        REQUIRE(str.find("x: i32") != std::string::npos);
        REQUIRE(str.find("y: i32") != std::string::npos);
    }

    SECTION("Anonymous struct toString") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("value"), i32Type));

        auto* anonStruct = registry.getStructType(intern(""), std::move(fields), emptyMethods(), flgNone, nullptr);
        auto str = anonStruct->toString();

        REQUIRE(str.find("struct") != std::string::npos);
        REQUIRE(str.find("value: i32") != std::string::npos);
    }

    SECTION("Empty struct toString") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};

        auto* emptyStruct = registry.getStructType(intern("Empty"), std::move(fields), emptyMethods(), flgNone, nullptr);
        auto str = emptyStruct->toString();

        REQUIRE(str.find("Empty") != std::string::npos);
        REQUIRE(str.find("{}") != std::string::npos);
    }

    SECTION("Packed struct toString") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("a"), charType));
        fields.emplace_back(std::make_pair(intern("b"), i32Type));

        auto* packedStruct = registry.getStructType(intern("Data"), std::move(fields), emptyMethods(), flgPacked, nullptr);
        auto str = packedStruct->toString();

        REQUIRE(str.find("packed") != std::string::npos);
    }
}

TEST_CASE_METHOD(StructTypeTestFixture, "StructType - Nested and Complex Types", "[types][struct]") {
    SECTION("Struct with array fields") {
        auto* arrayType = registry.getArrayType(i32Type, 5); // [5]i32

        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("data"), arrayType));
        fields.emplace_back(std::make_pair(intern("size"), i32Type));

        auto* arrayStruct = registry.getStructType(intern("ArrayStruct"), std::move(fields), emptyMethods(), flgNone, nullptr);

        REQUIRE(arrayStruct->hasField(intern("data")));
        REQUIRE(arrayStruct->hasField(intern("size")));
        REQUIRE(arrayStruct->getFieldType(intern("data")) == arrayType);
        REQUIRE(arrayStruct->hasStaticSize());

        // Size should be: array(5*4=20) + padding(0) + i32(4) = 24
        REQUIRE(arrayStruct->getStaticSize() == 24);
    }

    SECTION("Struct with tuple fields") {
        ArenaVector<const Type*> tupleElements{ArenaSTLAllocator<const Type*>(arena)};
        tupleElements.push_back(i32Type);
        tupleElements.push_back(f64Type);
        auto* tupleType = registry.getTupleType(std::move(tupleElements));

        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("coord"), tupleType));
        fields.emplace_back(std::make_pair(intern("id"), i32Type));

        auto* tupleStruct = registry.getStructType(intern("TupleStruct"), std::move(fields), emptyMethods(), flgNone, nullptr);

        REQUIRE(tupleStruct->hasField(intern("coord")));
        REQUIRE(tupleStruct->getFieldType(intern("coord")) == tupleType);
        REQUIRE(tupleStruct->hasStaticSize());
    }

    SECTION("Nested struct fields") {
        // Create inner struct first
        ArenaVector<std::pair<InternedString, const Type*>> innerFields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        innerFields.emplace_back(std::make_pair(intern("x"), i32Type));
        innerFields.emplace_back(std::make_pair(intern("y"), i32Type));
        auto* innerStruct = registry.getStructType(intern("Point"), std::move(innerFields), emptyMethods(), flgNone, nullptr);

        // Create outer struct with inner struct as field
        ArenaVector<std::pair<InternedString, const Type*>> outerFields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        outerFields.emplace_back(std::make_pair(intern("position"), innerStruct));
        outerFields.emplace_back(std::make_pair(intern("scale"), f64Type));

        auto* outerStruct = registry.getStructType(intern("Entity"), std::move(outerFields), emptyMethods(), flgNone, nullptr);

        REQUIRE(outerStruct->hasField(intern("position")));
        REQUIRE(outerStruct->getFieldType(intern("position")) == innerStruct);
        REQUIRE(outerStruct->hasStaticSize());
    }
}

TEST_CASE_METHOD(StructTypeTestFixture, "StructType - Type Classification", "[types][struct]") {
    ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
    fields.emplace_back(std::make_pair(intern("value"), i32Type));
    auto* testStruct = registry.getStructType(intern("TestStruct"), std::move(fields), emptyMethods(), flgNone, nullptr);

    SECTION("Type classification methods") {
        REQUIRE(!testStruct->isPrimitive());
        REQUIRE(testStruct->isComposite());
        REQUIRE(!testStruct->isCallable());
        REQUIRE(!testStruct->isNumeric());
        REQUIRE(!testStruct->isIntegral());
        REQUIRE(!testStruct->isFloatingPoint());
        REQUIRE(testStruct->hasStaticSize());
        REQUIRE(!testStruct->isDynamicallySized());
    }

    SECTION("Type conversion and assignment") {
        // Structs generally don't convert to other types
        REQUIRE(!testStruct->isAssignableFrom(i32Type));
        REQUIRE(!testStruct->isImplicitlyConvertibleTo(i32Type));
        REQUIRE(!testStruct->isExplicitlyConvertibleTo(i32Type));
        REQUIRE(!testStruct->isCompatibleWith(i32Type));

        // But should be assignable from itself
        REQUIRE(testStruct->isAssignableFrom(testStruct));
        REQUIRE(testStruct->isCompatibleWith(testStruct));
    }
}

TEST_CASE_METHOD(StructTypeTestFixture, "StructType - Edge Cases", "[types][struct]") {
    SECTION("Very large struct") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        for (int i = 0; i < 100; ++i) {
            fields.emplace_back(std::make_pair(
                intern("field" + std::to_string(i)),
                i % 2 == 0 ? static_cast<const Type*>(i32Type) : static_cast<const Type*>(i64Type)
            ));
        }

        auto* largeStruct = registry.getStructType(intern("Large"), std::move(fields), emptyMethods(), flgNone, nullptr);

        REQUIRE(largeStruct->getFieldCount() == 100);
        REQUIRE(largeStruct->hasField(intern("field0")));
        REQUIRE(largeStruct->hasField(intern("field99")));
        REQUIRE(!largeStruct->hasField(intern("field100")));
        REQUIRE(largeStruct->hasStaticSize());
        REQUIRE(largeStruct->getStaticSize() > 0);
    }

    SECTION("Struct with duplicate field names should be handled by registry") {
        // This test depends on how the registry validates input
        // For now, we assume the registry accepts it and the struct handles lookup
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("duplicate"), i32Type));
        fields.emplace_back(std::make_pair(intern("duplicate"), i64Type));

        auto* dupStruct = registry.getStructType(intern("Duplicate"), std::move(fields), emptyMethods(), flgNone, nullptr);

        // Should find the first occurrence
        REQUIRE(dupStruct->hasField(intern("duplicate")));
        REQUIRE(dupStruct->getFieldType(intern("duplicate")) == i32Type); // First one
        REQUIRE(dupStruct->getFieldIndex(intern("duplicate")) == 0); // First index
    }
}

TEST_CASE_METHOD(StructTypeTestFixture, "StructType - Method Support", "[types][struct]") {
    SECTION("Create struct with methods") {
        // Create function signatures for methods
        // say(this *Sayer, name string) -> void
        ArenaVector<const Type*> sayParams = makeArenaVector<const Type*>(arena);
        auto sayerPtrType = registry.getArrayType(charType, 1); // Placeholder for Sayer* type
        auto stringType = registry.getArrayType(charType, 10);   // Placeholder for string type
        sayParams.push_back(sayerPtrType);  // this parameter
        sayParams.push_back(stringType);    // name parameter
        auto saySignature = registry.getFunctionType(sayParams, registry.getVoidType());

        // const say(this *const Sayer, name string) -> bool (different return type to differentiate)
        ArenaVector<const Type*> constSayParams = makeArenaVector<const Type*>(arena);
        constSayParams.push_back(sayerPtrType);  // this parameter (const encoded in flags)
        constSayParams.push_back(stringType);    // name parameter
        auto constSaySignature = registry.getFunctionType(constSayParams, boolType); // Different return type

        // Create fields
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("data"), i32Type));

        // Create methods
        ArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>> methods{
            ArenaSTLAllocator<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(arena)
        };
        methods.emplace_back(std::make_tuple(intern("say"), saySignature, nullptr));
        methods.emplace_back(std::make_tuple(intern("say"), constSaySignature, nullptr)); // Overloaded const version

        auto* sayerStruct = registry.getStructType(intern("Sayer"), std::move(fields), std::move(methods), flgNone, nullptr);

        REQUIRE(sayerStruct != nullptr);
        REQUIRE(sayerStruct->getFieldCount() == 1);
        REQUIRE(sayerStruct->getMethodCount() == 2);

        // Test method access
        REQUIRE(sayerStruct->hasMethod(intern("say")));
        REQUIRE(!sayerStruct->hasMethod(intern("nonexistent")));

        // Test method lookup by name (should return both overloads)
        auto sayMethods = sayerStruct->getMethodsByName(intern("say"));
        REQUIRE(sayMethods.size() == 2);

        // Test specific method lookup by signature
        auto method1 = sayerStruct->getMethod(intern("say"), saySignature);
        auto method2 = sayerStruct->getMethod(intern("say"), constSaySignature);
        REQUIRE(method1 != nullptr);
        REQUIRE(method2 != nullptr);
        REQUIRE(method1 != method2);

        // Test method properties
        REQUIRE(method1->name == intern("say"));
        REQUIRE(method1->signature == saySignature);
        REQUIRE(method2->name == intern("say"));
        REQUIRE(method2->signature == constSaySignature);
    }

    SECTION("Struct with no methods") {
        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        fields.emplace_back(std::make_pair(intern("value"), i32Type));

        auto* simpleStruct = registry.getStructType(intern("Simple"), std::move(fields), emptyMethods(), flgNone, nullptr);

        REQUIRE(simpleStruct->getMethodCount() == 0);
        REQUIRE(!simpleStruct->hasMethod(intern("anything")));
        
        auto methods = simpleStruct->getMethodsByName(intern("anything"));
        REQUIRE(methods.size() == 0);
    }

    SECTION("Method toString representation") {
        // Create a simple method signature
        ArenaVector<const Type*> params = makeArenaVector<const Type*>(arena);
        params.push_back(i32Type);  // this parameter
        auto methodSignature = registry.getFunctionType(params, registry.getVoidType());

        ArenaVector<std::pair<InternedString, const Type*>> fields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        
        ArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>> methods{
            ArenaSTLAllocator<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(arena)
        };
        methods.emplace_back(std::make_tuple(intern("test"), methodSignature, nullptr));

        auto* methodStruct = registry.getStructType(intern("WithMethod"), std::move(fields), std::move(methods), flgNone, nullptr);
        auto str = methodStruct->toString();
        
        REQUIRE(str.find("WithMethod") != std::string::npos);
        REQUIRE(str.find("func test") != std::string::npos);
    }
}

TEST_CASE_METHOD(StructTypeTestFixture, "ClassType - Basic Inheritance and Virtual Methods", "[types][class]") {
    SECTION("Create class with inheritance") {
        // Create base class
        ArenaVector<std::pair<InternedString, const Type*>> baseFields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        baseFields.emplace_back(std::make_pair(intern("id"), i32Type));
        
        // Create virtual method for base class
        ArenaVector<const Type*> virtualParams = makeArenaVector<const Type*>(arena);
        virtualParams.push_back(charType);  // this parameter (placeholder)
        auto virtualSignature = registry.getFunctionType(virtualParams, registry.getVoidType());
        
        ArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>> baseMethods{
            ArenaSTLAllocator<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(arena)
        };
        baseMethods.emplace_back(std::make_tuple(intern("process"), virtualSignature, nullptr));
        
        // Create base class type (using ClassType constructor when it's added to TypeRegistry)
        // For now, create a simple class without base classes
        ArenaVector<std::pair<InternedString, const Type*>> derivedFields{ArenaSTLAllocator<std::pair<InternedString, const Type*>>(arena)};
        derivedFields.emplace_back(std::make_pair(intern("value"), i32Type));
        
        // This would be the API for creating class types (to be implemented in TypeRegistry)
        // auto* baseClass = registry.getClassType(intern("Base"), std::move(baseFields), std::move(baseMethods), {}, flgNone, nullptr);
        // auto* derivedClass = registry.getClassType(intern("Derived"), std::move(derivedFields), emptyMethods(), {baseClass}, flgNone, nullptr);
        
        // For now, just verify the struct system continues to work
        auto* testStruct = registry.getStructType(intern("TestClass"), std::move(derivedFields), std::move(baseMethods), flgNone, nullptr);
        
        REQUIRE(testStruct != nullptr);
        REQUIRE(testStruct->kind() == typStruct);
        REQUIRE(!testStruct->isValueType() == false);  // Structs are value types
        REQUIRE(!testStruct->supportsInheritance());   // Structs don't support inheritance
        REQUIRE(testStruct->getTypeKeyword() == "struct");
        
        // Verify method support works
        REQUIRE(testStruct->hasMethod(intern("process")));
        REQUIRE(testStruct->getMethodCount() == 1);
        auto processMethods = testStruct->getMethodsByName(intern("process"));
        REQUIRE(processMethods.size() == 1);
    }
}
