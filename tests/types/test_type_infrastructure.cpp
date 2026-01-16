#include "catch2.hpp"
#include <cxy/types.hpp>
#include <set>

using namespace cxy;

TEST_CASE("TypeKind enumeration", "[types][typekind]") {
    SECTION("All TypeKind enum values are defined") {
        // Test that all enum values exist and are distinct
        REQUIRE(typAuto != typInteger);
        REQUIRE(typInteger != typFloat);
        REQUIRE(typFloat != typBool);
        REQUIRE(typBool != typChar);
        REQUIRE(typChar != typVoid);
        REQUIRE(typVoid != typArray);
        REQUIRE(typArray != typTuple);
        REQUIRE(typTuple != typStruct);
        REQUIRE(typStruct != typClass);
        REQUIRE(typClass != typUnion);
        REQUIRE(typUnion != typFunction);
        REQUIRE(typFunction != typClosure);
        REQUIRE(typClosure != typGeneric);
        REQUIRE(typGeneric != typTypeAlias);
        REQUIRE(typTypeAlias != typUnknown);
    }

    SECTION("typeKindToString function works") {
        REQUIRE(std::string(typeKindToString(typAuto)) == "Auto");
        REQUIRE(std::string(typeKindToString(typInteger)) == "Integer");
        REQUIRE(std::string(typeKindToString(typFloat)) == "Float");
        REQUIRE(std::string(typeKindToString(typBool)) == "Bool");
        REQUIRE(std::string(typeKindToString(typChar)) == "Char");
        REQUIRE(std::string(typeKindToString(typVoid)) == "Void");
        REQUIRE(std::string(typeKindToString(typArray)) == "Array");
        REQUIRE(std::string(typeKindToString(typTuple)) == "Tuple");
        REQUIRE(std::string(typeKindToString(typStruct)) == "Struct");
        REQUIRE(std::string(typeKindToString(typClass)) == "Class");
        REQUIRE(std::string(typeKindToString(typUnion)) == "Union");
        REQUIRE(std::string(typeKindToString(typFunction)) == "Function");
        REQUIRE(std::string(typeKindToString(typClosure)) == "Closure");
        REQUIRE(std::string(typeKindToString(typGeneric)) == "Generic");
        REQUIRE(std::string(typeKindToString(typTypeAlias)) == "TypeAlias");
        REQUIRE(std::string(typeKindToString(typUnknown)) == "Unknown");
    }
}

TEST_CASE("TypeRegistry basic functionality", "[types][registry]") {
    SECTION("Singleton pattern works") {
        TypeRegistry& registry1 = TypeRegistry::instance();
        TypeRegistry& registry2 = TypeRegistry::instance();
        
        // Should return the same instance
        REQUIRE(&registry1 == &registry2);
    }

    SECTION("Basic registry operations") {
        TypeRegistry& registry = TypeRegistry::instance();
        
        // Initial state
        size_t initialCount = registry.getTypeCount();
        
        // Clear should reset count
        registry.clear();
        REQUIRE(registry.getTypeCount() == 0);
        
        // Restore initial state for other tests
        // (In Phase 1, we don't have actual types to register yet)
    }
}

TEST_CASE("Type utility structures", "[types][utilities]") {
    SECTION("TypeHash and TypeEqual exist and can be instantiated") {
        // These are used for hash tables containing types
        TypeHash hasher;
        TypeEqual equalizer;
        
        // Test with null pointers (edge case)
        REQUIRE(hasher(nullptr) == 0);
        REQUIRE(equalizer(nullptr, nullptr) == true);
        REQUIRE(equalizer(nullptr, reinterpret_cast<const Type*>(0x1)) == false);
        REQUIRE(equalizer(reinterpret_cast<const Type*>(0x1), nullptr) == false);
    }
}

TEST_CASE("CXY_TYPES macro expansion", "[types][macro]") {
    SECTION("Macro generates expected number of type kinds") {
        // Count the number of enum values by checking they're all different
        std::set<TypeKind> typeKinds = {
            typAuto, typInteger, typFloat, typBool, typChar, typVoid,
            typArray, typTuple, typStruct, typClass, typUnion,
            typFunction, typClosure, typGeneric, typTypeAlias, typUnknown
        };
        
        // Should have 16 distinct type kinds
        REQUIRE(typeKinds.size() == 16);
    }
}