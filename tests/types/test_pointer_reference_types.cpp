#include "catch2.hpp"
#include "cxy/types/registry.hpp"
#include "cxy/types/primitive.hpp"
#include "cxy/types/composite.hpp"
#include "cxy/arena_allocator.hpp"
#include "cxy/arena_stl.hpp"

using namespace cxy;

TEST_CASE("PointerType basic functionality", "[types][composite][pointer]") {
    SECTION("PointerType creation and basic properties") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        auto pointerType = registry.getPointerType(i32);
        
        REQUIRE(pointerType != nullptr);
        REQUIRE(pointerType->kind() == typPointer);
        REQUIRE(pointerType->getPointeeType() == i32);
        REQUIRE(pointerType->isPrimitive() == false);
        REQUIRE(pointerType->isComposite() == true);
        REQUIRE(pointerType->isCallable() == false);
        REQUIRE(pointerType->isNumeric() == false);
        REQUIRE(pointerType->isIntegral() == false);
        REQUIRE(pointerType->isFloatingPoint() == false);
    }

    SECTION("PointerType string representation") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();
        
        auto i32Ptr = registry.getPointerType(i32);
        auto f64Ptr = registry.getPointerType(f64);
        auto boolPtr = registry.getPointerType(boolType);
        
        REQUIRE(i32Ptr->toString() == "*i32");
        REQUIRE(f64Ptr->toString() == "*f64");
        REQUIRE(boolPtr->toString() == "*bool");
    }

    SECTION("PointerType size and alignment") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto pointerType = registry.getPointerType(i32);
        
        // Pointers have pointer size regardless of pointee type
        REQUIRE(pointerType->getStaticSize() == sizeof(void*));
        REQUIRE(pointerType->getAlignment() == sizeof(void*));
        REQUIRE(pointerType->hasStaticSize() == true);
        REQUIRE(pointerType->isDynamicallySized() == false);
    }

    SECTION("PointerType equality and hashing") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        
        auto i32Ptr1 = registry.getPointerType(i32);
        auto i32Ptr2 = registry.getPointerType(i32);
        auto f64Ptr = registry.getPointerType(f64);
        
        // Same pointee type should be equal
        REQUIRE(i32Ptr1->equals(i32Ptr2) == true);
        REQUIRE(i32Ptr1 == i32Ptr2); // Should be cached and return same instance
        
        // Different pointee types should not be equal
        REQUIRE(i32Ptr1->equals(f64Ptr) == false);
        REQUIRE(i32Ptr1->hash() != f64Ptr->hash());
    }

    SECTION("PointerType type relationships") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto i64 = registry.integerType(IntegerKind::I64);
        
        auto i32Ptr1 = registry.getPointerType(i32);
        auto i32Ptr2 = registry.getPointerType(i32);
        auto i64Ptr = registry.getPointerType(i64);
        
        // Same pointer types
        REQUIRE(i32Ptr1->isAssignableFrom(i32Ptr2) == true);
        REQUIRE(i32Ptr1->isCompatibleWith(i32Ptr2) == true);
        
        // Different pointer types - should not be directly assignable
        REQUIRE(i32Ptr1->isAssignableFrom(i64Ptr) == false);
        
        // Explicit conversion between different pointer types - now allowed if pointee types can convert
        REQUIRE(i32Ptr1->isExplicitlyConvertibleTo(i64Ptr) == true); // i32 can explicitly convert to i64
    }

    SECTION("Nested pointer types") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        auto i32Ptr = registry.getPointerType(i32);        // *i32
        auto i32PtrPtr = registry.getPointerType(i32Ptr);  // **i32
        auto i32PtrPtrPtr = registry.getPointerType(i32PtrPtr); // ***i32
        
        REQUIRE(i32Ptr->toString() == "*i32");
        REQUIRE(i32PtrPtr->toString() == "**i32");
        REQUIRE(i32PtrPtrPtr->toString() == "***i32");
        
        REQUIRE(i32PtrPtr->getPointeeType() == i32Ptr);
        REQUIRE(i32PtrPtrPtr->getPointeeType() == i32PtrPtr);
    }

    SECTION("PointerType with null pointee should return nullptr") {
        auto& registry = TypeRegistry::instance();
        auto nullPtr = registry.getPointerType(nullptr);
        
        REQUIRE(nullPtr == nullptr);
    }
}

TEST_CASE("ReferenceType basic functionality", "[types][composite][reference]") {
    SECTION("ReferenceType creation and basic properties") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        auto referenceType = registry.getReferenceType(i32);
        
        REQUIRE(referenceType != nullptr);
        REQUIRE(referenceType->kind() == typReference);
        REQUIRE(referenceType->getReferentType() == i32);
        REQUIRE(referenceType->isPrimitive() == false);
        REQUIRE(referenceType->isComposite() == true);
        REQUIRE(referenceType->isCallable() == false);
        REQUIRE(referenceType->isNumeric() == false);
        REQUIRE(referenceType->isIntegral() == false);
        REQUIRE(referenceType->isFloatingPoint() == false);
    }

    SECTION("ReferenceType string representation") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        auto boolType = registry.boolType();
        
        auto i32Ref = registry.getReferenceType(i32);
        auto f64Ref = registry.getReferenceType(f64);
        auto boolRef = registry.getReferenceType(boolType);
        
        REQUIRE(i32Ref->toString() == "&i32");
        REQUIRE(f64Ref->toString() == "&f64");
        REQUIRE(boolRef->toString() == "&bool");
    }

    SECTION("ReferenceType size and alignment") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto referenceType = registry.getReferenceType(i32);
        
        // References have pointer size regardless of referent type
        REQUIRE(referenceType->getStaticSize() == sizeof(void*));
        REQUIRE(referenceType->getAlignment() == sizeof(void*));
        REQUIRE(referenceType->hasStaticSize() == true);
        REQUIRE(referenceType->isDynamicallySized() == false);
    }

    SECTION("ReferenceType equality and hashing") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        
        auto i32Ref1 = registry.getReferenceType(i32);
        auto i32Ref2 = registry.getReferenceType(i32);
        auto f64Ref = registry.getReferenceType(f64);
        
        // Same referent type should be equal
        REQUIRE(i32Ref1->equals(i32Ref2) == true);
        REQUIRE(i32Ref1 == i32Ref2); // Should be cached and return same instance
        
        // Different referent types should not be equal
        REQUIRE(i32Ref1->equals(f64Ref) == false);
        REQUIRE(i32Ref1->hash() != f64Ref->hash());
    }

    SECTION("ReferenceType type relationships") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto i64 = registry.integerType(IntegerKind::I64);
        
        auto i32Ref1 = registry.getReferenceType(i32);
        auto i32Ref2 = registry.getReferenceType(i32);
        auto i64Ref = registry.getReferenceType(i64);
        
        // Same reference types
        REQUIRE(i32Ref1->isAssignableFrom(i32Ref2) == true);
        REQUIRE(i32Ref1->isCompatibleWith(i32Ref2) == true);
        
        // Different reference types - should not be directly assignable
        REQUIRE(i32Ref1->isAssignableFrom(i64Ref) == false);
        
        // Explicit conversion between different reference types - now allowed if referent types can convert
        REQUIRE(i32Ref1->isExplicitlyConvertibleTo(i64Ref) == true); // i32 can explicitly convert to i64
    }

    SECTION("Nested reference types (references to references)") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        auto i32Ref = registry.getReferenceType(i32);        // &i32
        auto i32RefRef = registry.getReferenceType(i32Ref);  // &&i32 (reference to reference)
        
        REQUIRE(i32Ref->toString() == "&i32");
        REQUIRE(i32RefRef->toString() == "&&i32");
        
        REQUIRE(i32RefRef->getReferentType() == i32Ref);
    }

    SECTION("ReferenceType with null referent should return nullptr") {
        auto& registry = TypeRegistry::instance();
        auto nullRef = registry.getReferenceType(nullptr);
        
        REQUIRE(nullRef == nullptr);
    }
}

TEST_CASE("PointerType vs ReferenceType distinctions", "[types][composite][pointer][reference]") {
    SECTION("Pointers and references to same type are different") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        auto i32Ptr = registry.getPointerType(i32);
        auto i32Ref = registry.getReferenceType(i32);
        
        // Different types entirely
        REQUIRE(static_cast<const Type*>(i32Ptr) != static_cast<const Type*>(i32Ref));
        REQUIRE(i32Ptr->equals(i32Ref) == false);
        REQUIRE(i32Ref->equals(i32Ptr) == false);
        REQUIRE(i32Ptr->kind() != i32Ref->kind());
        
        // Different string representations
        REQUIRE(i32Ptr->toString() == "*i32");
        REQUIRE(i32Ref->toString() == "&i32");
        
        // Different hashes
        REQUIRE(i32Ptr->hash() != i32Ref->hash());
    }

    SECTION("Mixed pointer/reference combinations") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        auto i32Ptr = registry.getPointerType(i32);      // *i32
        auto i32Ref = registry.getReferenceType(i32);    // &i32
        
        auto ptrToRef = registry.getPointerType(i32Ref); // Should resolve to *i32 (not *&i32)
        auto refToPtr = registry.getReferenceType(i32Ptr); // Should return nullptr - not allowed
        
        REQUIRE(ptrToRef->toString() == "*i32");  // Pointer to reference resolves to pointer to referent
        REQUIRE(refToPtr == nullptr);  // Reference to pointer is not allowed
        
        REQUIRE(ptrToRef->getPointeeType() == i32);  // Points to i32, not to the reference
        
        // ptrToRef should be the same as i32Ptr since pointer-to-reference resolves to pointer-to-referent
        REQUIRE(ptrToRef == i32Ptr);
        REQUIRE(ptrToRef->equals(i32Ptr) == true);
    }
}

TEST_CASE("PointerType and ReferenceType with composite types", "[types][composite][pointer][reference]") {
    SECTION("Pointers and references to arrays") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        auto arrayType = registry.getArrayType(i32, 10); // [10]i32
        auto arrayPtr = registry.getPointerType(arrayType); // *[10]i32
        auto arrayRef = registry.getReferenceType(arrayType); // &[10]i32
        
        REQUIRE(arrayPtr->toString() == "*[10]i32");
        REQUIRE(arrayRef->toString() == "&[10]i32");
        
        REQUIRE(arrayPtr->getPointeeType() == arrayType);
        REQUIRE(arrayRef->getReferentType() == arrayType);
    }

    SECTION("Pointers and references to function types") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto voidType = registry.voidType();
        
        ArenaVector<const Type*> params = makeArenaVector<const Type*>(arena);
        params.push_back(i32);
        
        auto funcType = registry.getFunctionType(params, voidType); // (i32) -> void
        auto funcPtr = registry.getPointerType(funcType);           // *(i32) -> void
        auto funcRef = registry.getReferenceType(funcType);         // &(i32) -> void (allowed - function types are immutable)
        
        REQUIRE(funcPtr->getPointeeType() == funcType);
        REQUIRE(funcRef != nullptr);  // Function references are allowed
        REQUIRE(funcRef->getReferentType() == funcType);
        
        // Only the function type itself should be callable
        REQUIRE(funcPtr->isCallable() == false); // The pointer itself is not callable
        REQUIRE(funcRef->isCallable() == false); // The reference itself is not callable
        REQUIRE(funcType->isCallable() == true);  // But the function type is
    }
}

TEST_CASE("PointerType and ReferenceType type registry caching", "[types][composite][pointer][reference][registry]") {
    SECTION("Registry properly caches pointer types") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        auto ptr1 = registry.getPointerType(i32);
        auto ptr2 = registry.getPointerType(i32);
        auto ptr3 = registry.getPointerType(i32);
        
        // Should return the same cached instance
        REQUIRE(ptr1 == ptr2);
        REQUIRE(ptr2 == ptr3);
        REQUIRE(ptr1 == ptr3);
    }

    SECTION("Registry properly caches reference types") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        auto ref1 = registry.getReferenceType(i32);
        auto ref2 = registry.getReferenceType(i32);
        auto ref3 = registry.getReferenceType(i32);
        
        // Should return the same cached instance
        REQUIRE(ref1 == ref2);
        REQUIRE(ref2 == ref3);
        REQUIRE(ref1 == ref3);
    }

    SECTION("Registry caches different types separately") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        
        auto i32Ptr = registry.getPointerType(i32);
        auto f64Ptr = registry.getPointerType(f64);
        auto i32Ref = registry.getReferenceType(i32);
        auto f64Ref = registry.getReferenceType(f64);
        
        // All should be different instances
        std::set<const void*> addresses = {
            i32Ptr, f64Ptr, i32Ref, f64Ref
        };
        REQUIRE(addresses.size() == 4); // All unique addresses
    }

    SECTION("Type count includes pointer and reference types") {
        auto& registry = TypeRegistry::instance();
        
        // Simply verify that creating pointer and reference types increases count
        auto i32 = registry.integerType(IntegerKind::I32);
        auto beforePtr = registry.getTypeCount();
        
        auto i32Ptr = registry.getPointerType(i32);
        auto afterPtr = registry.getTypeCount();
        
        auto i32Ref = registry.getReferenceType(i32);
        auto afterRef = registry.getTypeCount();
        
        // Each new type should increase the count
        REQUIRE(afterPtr >= beforePtr);
        REQUIRE(afterRef >= afterPtr);
    }
}

TEST_CASE("PointerType and ReferenceType edge cases", "[types][composite][pointer][reference][edge-cases]") {
    SECTION("Deeply nested pointer types") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        const Type* currentType = i32;
        std::string expectedString = "i32";
        
        // Create 5 levels of pointer nesting
        for (int i = 0; i < 5; ++i) {
            currentType = registry.getPointerType(currentType);
            expectedString = "*" + expectedString;
        }
        
        REQUIRE(currentType->toString() == "*****i32");
        REQUIRE(currentType->kind() == typPointer);
    }

    SECTION("Deeply nested reference types") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        const Type* currentType = i32;
        std::string expectedString = "i32";
        
        // Create 3 levels of reference nesting
        for (int i = 0; i < 3; ++i) {
            currentType = registry.getReferenceType(currentType);
            expectedString = "&" + expectedString;
        }
        
        REQUIRE(currentType->toString() == "&&&i32");
        REQUIRE(currentType->kind() == typReference);
    }

    SECTION("Complex mixed nesting") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        // Create complex nesting, but reference-to-pointer should be forbidden
        auto i32Ref = registry.getReferenceType(i32);        // &i32
        auto ptrToRef = registry.getPointerType(i32Ref);     // Should resolve to *i32
        auto refToPtr = registry.getReferenceType(ptrToRef); // Should return nullptr - not allowed
        
        REQUIRE(ptrToRef->toString() == "*i32");  // Pointer to reference resolves to pointer to referent
        REQUIRE(refToPtr == nullptr);  // Reference to pointer is not allowed
        
        // Verify the resolution behavior
        REQUIRE(ptrToRef == registry.getPointerType(i32));  // Should be same as direct *i32
    }
}

TEST_CASE("PointerType and ReferenceType string representation in type system", "[types][composite][pointer][reference][kind]") {
    SECTION("typeKindToString works for new types") {
        REQUIRE(std::string(typeKindToString(typPointer)) == "Pointer");
        REQUIRE(std::string(typeKindToString(typReference)) == "Reference");
    }

    SECTION("Type kinds are distinct") {
        REQUIRE(typPointer != typReference);
        REQUIRE(typPointer != typArray);
        REQUIRE(typReference != typArray);
        REQUIRE(typPointer != typInteger);
        REQUIRE(typReference != typInteger);
    }
}

TEST_CASE("PointerType resolves pointer-to-reference to pointer-to-referent", "[types][composite][pointer][reference][resolution]") {
    SECTION("Pointer to reference resolves to pointer to referent type") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        
        auto i32Ref = registry.getReferenceType(i32);    // &i32
        auto f64Ref = registry.getReferenceType(f64);    // &f64
        
        auto ptrToI32Ref = registry.getPointerType(i32Ref);  // Should resolve to *i32
        auto ptrToF64Ref = registry.getPointerType(f64Ref);  // Should resolve to *f64
        
        auto directI32Ptr = registry.getPointerType(i32);    // *i32
        auto directF64Ptr = registry.getPointerType(f64);    // *f64
        
        // Pointer-to-reference should resolve to the same as direct pointer
        REQUIRE(ptrToI32Ref == directI32Ptr);
        REQUIRE(ptrToF64Ref == directF64Ptr);
        
        // String representations should be simple pointer types
        REQUIRE(ptrToI32Ref->toString() == "*i32");
        REQUIRE(ptrToF64Ref->toString() == "*f64");
        
        // Pointee types should be the original types, not the references
        REQUIRE(ptrToI32Ref->getPointeeType() == i32);
        REQUIRE(ptrToF64Ref->getPointeeType() == f64);
    }
    
    SECTION("Multiple levels of pointer-to-reference resolution") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        auto i32Ref = registry.getReferenceType(i32);           // &i32
        auto i32RefRef = registry.getReferenceType(i32Ref);     // &&i32
        auto i32RefRefRef = registry.getReferenceType(i32RefRef); // &&&i32
        
        // Taking pointers to nested references should all resolve to *i32
        auto ptrToRef = registry.getPointerType(i32Ref);        // *(&i32) -> *i32
        auto ptrToRefRef = registry.getPointerType(i32RefRef);  // *(&&i32) -> *i32
        auto ptrToRefRefRef = registry.getPointerType(i32RefRefRef); // *(&&&i32) -> *i32
        
        auto directPtr = registry.getPointerType(i32);
        
        // All should resolve to the same direct pointer
        REQUIRE(ptrToRef == directPtr);
        REQUIRE(ptrToRefRef == directPtr);
        REQUIRE(ptrToRefRefRef == directPtr);
        
        REQUIRE(ptrToRef->toString() == "*i32");
        REQUIRE(ptrToRefRef->toString() == "*i32");
        REQUIRE(ptrToRefRefRef->toString() == "*i32");
    }
    
    SECTION("Pointer-to-reference in composite types") {
        ArenaAllocator arena(1024);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        auto i32Ref = registry.getReferenceType(i32);  // &i32
        auto arrayOfRef = registry.getArrayType(i32Ref, 10);  // [10]&i32
        
        // Taking pointer to array of references should work normally
        auto ptrToArrayOfRef = registry.getPointerType(arrayOfRef);  // *[10]&i32
        
        REQUIRE(ptrToArrayOfRef->toString() == "*[10]&i32");
        REQUIRE(ptrToArrayOfRef->getPointeeType() == arrayOfRef);
        
        // But taking pointer to reference directly should resolve
        auto ptrToRef = registry.getPointerType(i32Ref);  // *(&i32) -> *i32
        REQUIRE(ptrToRef->toString() == "*i32");
        REQUIRE(ptrToRef->getPointeeType() == i32);
    }
}

TEST_CASE("Reference to pointer is forbidden", "[types][composite][pointer][reference][forbidden]") {
    SECTION("getReferenceType returns nullptr for pointer types") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto f64 = registry.floatType(FloatKind::F64);
        
        auto i32Ptr = registry.getPointerType(i32);
        auto f64Ptr = registry.getPointerType(f64);
        
        // References to pointers should not be allowed
        auto refToI32Ptr = registry.getReferenceType(i32Ptr);
        auto refToF64Ptr = registry.getReferenceType(f64Ptr);
        
        REQUIRE(refToI32Ptr == nullptr);
        REQUIRE(refToF64Ptr == nullptr);
    }
    
    SECTION("No references to nested pointer types") {
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        
        auto i32Ptr = registry.getPointerType(i32);        // *i32
        auto i32PtrPtr = registry.getPointerType(i32Ptr);  // **i32
        
        // References to any level of pointer should be forbidden
        auto refToPtr = registry.getReferenceType(i32Ptr);
        auto refToPtrPtr = registry.getReferenceType(i32PtrPtr);
        
        REQUIRE(refToPtr == nullptr);
        REQUIRE(refToPtrPtr == nullptr);
    }
    
    SECTION("References to non-pointer types are still allowed") {
        ArenaAllocator arena(512);
        auto& registry = TypeRegistry::instance();
        auto i32 = registry.integerType(IntegerKind::I32);
        auto boolType = registry.boolType();
        
        auto params = makeArenaVector<const Type*>(arena);
        params.push_back(i32);
        auto funcType = registry.getFunctionType(params, boolType);
        
        auto i32Ref = registry.getReferenceType(i32);
        auto boolRef = registry.getReferenceType(boolType);
        auto funcRef = registry.getReferenceType(funcType);
        
        // These should all be allowed
        REQUIRE(i32Ref != nullptr);
        REQUIRE(boolRef != nullptr);
        REQUIRE(funcRef != nullptr);
        
        REQUIRE(i32Ref->toString() == "&i32");
        REQUIRE(boolRef->toString() == "&bool");
        REQUIRE(funcRef->toString() == "&(i32) -> bool");
    }
}

// Helper class for inheritance tests
class InheritanceTestFixture {
public:
    InheritanceTestFixture() : interner(arena) {}
    
    InternedString intern(const std::string& str) {
        return interner.intern(str);
    }
    
    auto emptyFields() {
        return makeArenaVector<std::pair<InternedString, const Type*>>(arena);
    }
    
    auto emptyMethods() {
        return makeArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(arena);
    }
    
    ArenaAllocator arena{1024};
    StringInterner interner;
    TypeRegistry& registry = TypeRegistry::instance();
};

TEST_CASE("PointerType inheritance-aware conversions", "[types][composite][pointer][inheritance]") {
    SECTION("Pointer assignment with class inheritance") {
        InheritanceTestFixture fixture;
        
        // Create base class
        auto baseFields = fixture.emptyFields();
        auto baseMethods = fixture.emptyMethods();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("Base"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       nullptr, // no base class
                                                       flgNone, 
                                                       nullptr);
        
        // Create derived class
        auto derivedFields = fixture.emptyFields();
        auto derivedMethods = fixture.emptyMethods();
        
        auto derivedClass = fixture.registry.getClassType(fixture.intern("Derived"), 
                                                          std::move(derivedFields), 
                                                          std::move(derivedMethods),
                                                          baseClass, // inherit from base
                                                          flgNone, 
                                                          nullptr);
        
        auto basePtr = fixture.registry.getPointerType(baseClass);
        auto derivedPtr = fixture.registry.getPointerType(derivedClass);
        
        // Test pointer assignment compatibility
        // Base pointer should be assignable from derived pointer (polymorphic assignment)
        REQUIRE(basePtr->isAssignableFrom(derivedPtr) == true);
        
        // Derived pointer should NOT be assignable from base pointer (unsafe downcast)
        REQUIRE(derivedPtr->isAssignableFrom(basePtr) == false);
        
        // Both should be assignable from themselves
        REQUIRE(basePtr->isAssignableFrom(basePtr) == true);
        REQUIRE(derivedPtr->isAssignableFrom(derivedPtr) == true);
    }
    
    SECTION("Pointer implicit conversion with class inheritance") {
        InheritanceTestFixture fixture;
        
        // Create inheritance chain: A -> B -> C
        auto fieldsA = fixture.emptyFields();
        auto methodsA = fixture.emptyMethods();
        
        auto classA = fixture.registry.getClassType(fixture.intern("A"), 
                                                    std::move(fieldsA), 
                                                    std::move(methodsA),
                                                    nullptr,
                                                    flgNone, 
                                                    nullptr);
        
        auto fieldsB = fixture.emptyFields();
        auto methodsB = fixture.emptyMethods();
        
        auto classB = fixture.registry.getClassType(fixture.intern("B"), 
                                                    std::move(fieldsB), 
                                                    std::move(methodsB),
                                                    classA,
                                                    flgNone, 
                                                    nullptr);
        
        auto fieldsC = fixture.emptyFields();
        auto methodsC = fixture.emptyMethods();
        
        auto classC = fixture.registry.getClassType(fixture.intern("C"), 
                                                    std::move(fieldsC), 
                                                    std::move(methodsC),
                                                    classB,
                                                    flgNone, 
                                                    nullptr);
        
        auto ptrA = fixture.registry.getPointerType(classA);
        auto ptrB = fixture.registry.getPointerType(classB);
        auto ptrC = fixture.registry.getPointerType(classC);
        
        // Test implicit upcasting (safe)
        REQUIRE(ptrB->isImplicitlyConvertibleTo(ptrA) == true);   // *B -> *A
        REQUIRE(ptrC->isImplicitlyConvertibleTo(ptrA) == true);   // *C -> *A (transitive)
        REQUIRE(ptrC->isImplicitlyConvertibleTo(ptrB) == true);   // *C -> *B
        
        // Test implicit downcasting (should be forbidden)
        REQUIRE(ptrA->isImplicitlyConvertibleTo(ptrB) == false);  // *A -> *B (unsafe)
        REQUIRE(ptrA->isImplicitlyConvertibleTo(ptrC) == false);  // *A -> *C (unsafe)
        REQUIRE(ptrB->isImplicitlyConvertibleTo(ptrC) == false);  // *B -> *C (unsafe)
        
        // Test implicit conversion to self (via equals)
        REQUIRE(ptrA->isImplicitlyConvertibleTo(ptrA) == true);   // Identity
        REQUIRE(ptrB->isImplicitlyConvertibleTo(ptrB) == true);   
        REQUIRE(ptrC->isImplicitlyConvertibleTo(ptrC) == true);
    }
    
    SECTION("Pointer explicit conversion with class inheritance") {
        InheritanceTestFixture fixture;
        
        // Create simple inheritance: Base -> Derived
        auto baseFields = fixture.emptyFields();
        auto baseMethods = fixture.emptyMethods();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("Base"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       nullptr,
                                                       flgNone, 
                                                       nullptr);
        
        auto derivedFields = fixture.emptyFields();
        auto derivedMethods = fixture.emptyMethods();
        
        auto derivedClass = fixture.registry.getClassType(fixture.intern("Derived"), 
                                                          std::move(derivedFields), 
                                                          std::move(derivedMethods),
                                                          baseClass,
                                                          flgNone, 
                                                          nullptr);
        
        auto basePtr = fixture.registry.getPointerType(baseClass);
        auto derivedPtr = fixture.registry.getPointerType(derivedClass);
        
        // Test explicit upcasting (safe, should be allowed)
        REQUIRE(derivedPtr->isExplicitlyConvertibleTo(basePtr) == true);
        
        // Test explicit downcasting (potentially unsafe, but allowed with explicit cast)
        REQUIRE(basePtr->isExplicitlyConvertibleTo(derivedPtr) == true);
        
        // Test explicit conversion to self
        REQUIRE(basePtr->isExplicitlyConvertibleTo(basePtr) == true);
        REQUIRE(derivedPtr->isExplicitlyConvertibleTo(derivedPtr) == true);
    }
}

TEST_CASE("ReferenceType inheritance-aware conversions", "[types][composite][reference][inheritance]") {
    SECTION("Reference assignment with class inheritance") {
        InheritanceTestFixture fixture;
        
        // Create base class
        auto baseFields = fixture.emptyFields();
        auto baseMethods = fixture.emptyMethods();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("Base"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       nullptr,
                                                       flgNone, 
                                                       nullptr);
        
        // Create derived class
        auto derivedFields = fixture.emptyFields();
        auto derivedMethods = fixture.emptyMethods();
        
        auto derivedClass = fixture.registry.getClassType(fixture.intern("Derived"), 
                                                          std::move(derivedFields), 
                                                          std::move(derivedMethods),
                                                          baseClass,
                                                          flgNone, 
                                                          nullptr);
        
        auto baseRef = fixture.registry.getReferenceType(baseClass);
        auto derivedRef = fixture.registry.getReferenceType(derivedClass);
        
        // Test reference assignment compatibility
        // Base reference should be assignable from derived reference (polymorphic assignment)
        REQUIRE(baseRef->isAssignableFrom(derivedRef) == true);
        
        // Derived reference should NOT be assignable from base reference (unsafe downcast)
        REQUIRE(derivedRef->isAssignableFrom(baseRef) == false);
        
        // Both should be assignable from themselves
        REQUIRE(baseRef->isAssignableFrom(baseRef) == true);
        REQUIRE(derivedRef->isAssignableFrom(derivedRef) == true);
    }
    
    SECTION("Reference implicit conversion with class inheritance") {
        InheritanceTestFixture fixture;
        
        // Create inheritance chain: A -> B -> C
        auto fieldsA = fixture.emptyFields();
        auto methodsA = fixture.emptyMethods();
        
        auto classA = fixture.registry.getClassType(fixture.intern("A"), 
                                                    std::move(fieldsA), 
                                                    std::move(methodsA),
                                                    nullptr,
                                                    flgNone, 
                                                    nullptr);
        
        auto fieldsB = fixture.emptyFields();
        auto methodsB = fixture.emptyMethods();
        
        auto classB = fixture.registry.getClassType(fixture.intern("B"), 
                                                    std::move(fieldsB), 
                                                    std::move(methodsB),
                                                    classA,
                                                    flgNone, 
                                                    nullptr);
        
        auto fieldsC = fixture.emptyFields();
        auto methodsC = fixture.emptyMethods();
        
        auto classC = fixture.registry.getClassType(fixture.intern("C"), 
                                                    std::move(fieldsC), 
                                                    std::move(methodsC),
                                                    classB,
                                                    flgNone, 
                                                    nullptr);
        
        auto refA = fixture.registry.getReferenceType(classA);
        auto refB = fixture.registry.getReferenceType(classB);
        auto refC = fixture.registry.getReferenceType(classC);
        
        // Test implicit upcasting (safe)
        REQUIRE(refB->isImplicitlyConvertibleTo(refA) == true);   // &B -> &A
        REQUIRE(refC->isImplicitlyConvertibleTo(refA) == true);   // &C -> &A (transitive)
        REQUIRE(refC->isImplicitlyConvertibleTo(refB) == true);   // &C -> &B
        
        // Test implicit downcasting (should be forbidden)
        REQUIRE(refA->isImplicitlyConvertibleTo(refB) == false);  // &A -> &B (unsafe)
        REQUIRE(refA->isImplicitlyConvertibleTo(refC) == false);  // &A -> &C (unsafe)
        REQUIRE(refB->isImplicitlyConvertibleTo(refC) == false);  // &B -> &C (unsafe)
        
        // Test implicit conversion to self (via equals)
        REQUIRE(refA->isImplicitlyConvertibleTo(refA) == true);   // Identity
        REQUIRE(refB->isImplicitlyConvertibleTo(refB) == true);   
        REQUIRE(refC->isImplicitlyConvertibleTo(refC) == true);
    }
    
    SECTION("Reference explicit conversion with class inheritance") {
        InheritanceTestFixture fixture;
        
        // Create simple inheritance: Base -> Derived
        auto baseFields = fixture.emptyFields();
        auto baseMethods = fixture.emptyMethods();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("Base"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       nullptr,
                                                       flgNone, 
                                                       nullptr);
        
        auto derivedFields = fixture.emptyFields();
        auto derivedMethods = fixture.emptyMethods();
        
        auto derivedClass = fixture.registry.getClassType(fixture.intern("Derived"), 
                                                          std::move(derivedFields), 
                                                          std::move(derivedMethods),
                                                          baseClass,
                                                          flgNone, 
                                                          nullptr);
        
        auto baseRef = fixture.registry.getReferenceType(baseClass);
        auto derivedRef = fixture.registry.getReferenceType(derivedClass);
        
        // Test explicit upcasting (safe, should be allowed)
        REQUIRE(derivedRef->isExplicitlyConvertibleTo(baseRef) == true);
        
        // Test explicit downcasting (potentially unsafe, but allowed with explicit cast)
        REQUIRE(baseRef->isExplicitlyConvertibleTo(derivedRef) == true);
        
        // Test explicit conversion to self
        REQUIRE(baseRef->isExplicitlyConvertibleTo(baseRef) == true);
        REQUIRE(derivedRef->isExplicitlyConvertibleTo(derivedRef) == true);
    }
}