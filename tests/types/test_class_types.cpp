#include "catch2.hpp"
#include "cxy/types/registry.hpp"
#include "cxy/types/primitive.hpp"
#include "cxy/types/composite.hpp"
#include "cxy/arena_allocator.hpp"
#include "cxy/arena_stl.hpp"
#include "cxy/strings.hpp"

using namespace cxy;

namespace {

// Test fixture for class type tests
class ClassTypeTestFixture {
public:
    ClassTypeTestFixture() : interner(arena) {
        // Get basic primitive types
        i32Type = registry.integerType(IntegerKind::I32);
        f64Type = registry.floatType(FloatKind::F64);
        boolType = registry.boolType();
        voidType = registry.voidType();
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

    // Helper for no base class
    const ClassType* noBaseClass() {
        return nullptr;
    }

    TypeRegistry& registry = TypeRegistry::instance();
    ArenaAllocator arena;
    StringInterner interner;

    // Common types
    const IntegerType* i32Type;
    const FloatType* f64Type;
    const BoolType* boolType;
    const VoidType* voidType;
};

} // namespace

TEST_CASE("ClassType basic functionality", "[types][composite][class]") {
    SECTION("ClassType creation and basic properties") {
        ClassTypeTestFixture fixture;
        
        // Create basic class with fields
        auto fields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        fields.emplace_back(fixture.intern("x"), fixture.i32Type);
        fields.emplace_back(fixture.intern("y"), fixture.i32Type);
        
        auto methods = fixture.emptyMethods();
        auto baseClass = fixture.noBaseClass();
        
        auto classType = fixture.registry.getClassType(fixture.intern("Point"), 
                                                       std::move(fields), 
                                                       std::move(methods),
                                                       baseClass,
                                                       flgNone, 
                                                       nullptr);
        
        REQUIRE(classType != nullptr);
        REQUIRE(classType->kind() == typClass);
        REQUIRE(classType->getName().view() == "Point");
        REQUIRE(classType->getFieldCount() == 2);
        REQUIRE(classType->getMethodCount() == 0);
        REQUIRE(classType->hasBaseClass() == false);
        
        // Type classification
        REQUIRE(classType->isPrimitive() == false);
        REQUIRE(classType->isComposite() == true);
        REQUIRE(classType->isValueType() == false);        // Classes have reference semantics
        REQUIRE(classType->supportsInheritance() == true);
        REQUIRE(classType->getTypeKeyword() == "class");
        REQUIRE(classType->isCallable() == false);
        REQUIRE(classType->isNumeric() == false);
        REQUIRE(classType->isIntegral() == false);
        REQUIRE(classType->isFloatingPoint() == false);
    }

    SECTION("ClassType string representation") {
        ClassTypeTestFixture fixture;
        
        auto fields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        fields.emplace_back(fixture.intern("value"), fixture.i32Type);
        
        auto methods = fixture.emptyMethods();
        auto baseClass = fixture.noBaseClass();
        
        auto classType = fixture.registry.getClassType(fixture.intern("TestClass"), 
                                                       std::move(fields), 
                                                       std::move(methods),
                                                       baseClass,
                                                       flgNone, 
                                                       nullptr);
        
        REQUIRE(classType->toString().find("TestClass") != std::string::npos);
        
        // Test anonymous class shows type keyword
        auto fieldsAnon = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methodsAnon = fixture.emptyMethods();
        auto baseClassAnon = fixture.noBaseClass();
        
        auto anonClass = fixture.registry.getClassType(fixture.intern(""), 
                                                       std::move(fieldsAnon), 
                                                       std::move(methodsAnon),
                                                       baseClassAnon,
                                                       flgNone, 
                                                       nullptr);
        
        REQUIRE(anonClass->toString().find("class") != std::string::npos);
    }

    SECTION("ClassType size and alignment") {
        ClassTypeTestFixture fixture;
        
        auto fields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methods = fixture.emptyMethods();
        auto baseClass = fixture.noBaseClass();
        
        auto classType = fixture.registry.getClassType(fixture.intern("RefClass"), 
                                                       std::move(fields), 
                                                       std::move(methods),
                                                       baseClass,
                                                       flgNone, 
                                                       nullptr);
        
        // Classes have reference semantics - size is pointer size
        REQUIRE(classType->getStaticSize() == sizeof(void*));
        REQUIRE(classType->getAlignment() == alignof(void*));
        REQUIRE(classType->hasStaticSize() == true);
        REQUIRE(classType->isDynamicallySized() == false);
    }

    SECTION("ClassType with methods") {
        ClassTypeTestFixture fixture;
        
        // Create method signature
        auto params = makeArenaVector<const Type*>(fixture.arena);
        params.push_back(fixture.i32Type);
        auto methodSig = fixture.registry.getFunctionType(params, fixture.voidType);
        
        auto fields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methods = makeArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(fixture.arena);
        methods.emplace_back(fixture.intern("setValue"), methodSig, nullptr);
        
        auto baseClass = fixture.noBaseClass();
        
        auto classType = fixture.registry.getClassType(fixture.intern("Container"), 
                                                       std::move(fields), 
                                                       std::move(methods),
                                                       baseClass,
                                                       flgNone, 
                                                       nullptr);
        
        REQUIRE(classType->getMethodCount() == 1);
        REQUIRE(classType->hasMethod(fixture.intern("setValue")) == true);
        REQUIRE(classType->hasMethod(fixture.intern("nonExistent")) == false);
        
        auto foundMethod = classType->getMethod(fixture.intern("setValue"), methodSig);
        REQUIRE(foundMethod != nullptr);
        REQUIRE(foundMethod->name.view() == "setValue");
        REQUIRE(foundMethod->signature == methodSig);
    }
}

TEST_CASE("ClassType inheritance", "[types][composite][class][inheritance]") {
    SECTION("Single inheritance") {
        ClassTypeTestFixture fixture;
        
        // Create base class
        auto baseFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        baseFields.emplace_back(fixture.intern("id"), fixture.i32Type);
        
        auto baseMethods = fixture.emptyMethods();
        auto baseBaseClass = fixture.noBaseClass();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("Base"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       baseBaseClass,
                                                       flgNone, 
                                                       nullptr);
        
        // Create derived class
        auto derivedFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        derivedFields.emplace_back(fixture.intern("value"), fixture.f64Type);
        
        auto derivedMethods = fixture.emptyMethods();
        auto derivedBaseClass = baseClass;
        
        auto derivedClass = fixture.registry.getClassType(fixture.intern("Derived"), 
                                                          std::move(derivedFields), 
                                                          std::move(derivedMethods),
                                                          derivedBaseClass,
                                                          flgNone, 
                                                          nullptr);
        
        REQUIRE(derivedClass->hasBaseClass() == true);
        REQUIRE(derivedClass->getBaseClass() == baseClass);
        
        // Test inheritance relationships
        REQUIRE(baseClass->isBaseOf(derivedClass) == true);
        REQUIRE(derivedClass->isDerivedFrom(baseClass) == true);
        REQUIRE(derivedClass->isBaseOf(baseClass) == false);
        REQUIRE(baseClass->isDerivedFrom(derivedClass) == false);
        
        // Test field access (derived should see base fields)
        REQUIRE(derivedClass->hasField(fixture.intern("id")) == true);    // From base
        REQUIRE(derivedClass->hasField(fixture.intern("value")) == true); // From derived
    }

    SECTION("Inheritance chain") {
        ClassTypeTestFixture fixture;
        
        // Create chain: Base -> Intermediate -> Final
        auto baseFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        baseFields.emplace_back(fixture.intern("x"), fixture.i32Type);
        
        auto baseMethods = fixture.emptyMethods();
        auto baseBaseClass = fixture.noBaseClass();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("BaseClass"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       baseBaseClass,
                                                       flgNone, 
                                                       nullptr);
        
        // Create intermediate class
        auto intermediateFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        intermediateFields.emplace_back(fixture.intern("y"), fixture.f64Type);
        
        auto intermediateMethods = fixture.emptyMethods();
        auto intermediateBaseClass = baseClass;
        
        auto intermediateClass = fixture.registry.getClassType(fixture.intern("IntermediateClass"), 
                                                               std::move(intermediateFields), 
                                                               std::move(intermediateMethods),
                                                               intermediateBaseClass,
                                                               flgNone, 
                                                               nullptr);
        
        // Create final derived class
        auto finalFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        finalFields.emplace_back(fixture.intern("z"), fixture.boolType);
        auto finalMethods = fixture.emptyMethods();
        auto finalBaseClass = intermediateClass;
        
        auto finalClass = fixture.registry.getClassType(fixture.intern("FinalClass"), 
                                                        std::move(finalFields), 
                                                        std::move(finalMethods),
                                                        finalBaseClass,
                                                        flgNone, 
                                                        nullptr);
        
        REQUIRE(baseClass->isBaseOf(intermediateClass) == true);
        REQUIRE(baseClass->isBaseOf(finalClass) == true);
        REQUIRE(intermediateClass->isBaseOf(finalClass) == true);
        REQUIRE(finalClass->isDerivedFrom(baseClass) == true);
        REQUIRE(finalClass->isDerivedFrom(intermediateClass) == true);
    }

    SECTION("Deep inheritance chain") {
        ClassTypeTestFixture fixture;
        
        // Create inheritance chain: A -> B -> C
        auto fieldsA = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methodsA = fixture.emptyMethods();
        auto baseClassA = fixture.noBaseClass();
        
        auto classA = fixture.registry.getClassType(fixture.intern("A"), 
                                                    std::move(fieldsA), 
                                                    std::move(methodsA),
                                                    baseClassA,
                                                    flgNone, 
                                                    nullptr);
        
        auto fieldsB = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methodsB = fixture.emptyMethods();
        auto baseClassB = classA;
        
        auto classB = fixture.registry.getClassType(fixture.intern("B"), 
                                                    std::move(fieldsB), 
                                                    std::move(methodsB),
                                                    baseClassB,
                                                    flgNone, 
                                                    nullptr);
        
        auto fieldsC = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methodsC = fixture.emptyMethods();
        auto baseClassC = classB;
        
        auto classC = fixture.registry.getClassType(fixture.intern("C"), 
                                                    std::move(fieldsC), 
                                                    std::move(methodsC),
                                                    baseClassC,
                                                    flgNone, 
                                                    nullptr);
        
        // Test deep inheritance relationships
        REQUIRE(classA->isBaseOf(classB) == true);
        REQUIRE(classA->isBaseOf(classC) == true);  // Transitive
        REQUIRE(classB->isBaseOf(classC) == true);
        
        REQUIRE(classC->isDerivedFrom(classA) == true);  // Transitive
        REQUIRE(classC->isDerivedFrom(classB) == true);
        REQUIRE(classB->isDerivedFrom(classA) == true);
        
        // Test common base finding
        // B and C both derive from A, but B is also a direct base of C
        auto commonBase = classB->findCommonBase(classC);
        REQUIRE(commonBase == classB);  // B is the most immediate common base
        
        // A and C should have A as common base
        auto commonBaseAC = classA->findCommonBase(classC);
        REQUIRE(commonBaseAC == classA);
    }
}

TEST_CASE("ClassType virtual methods", "[types][composite][class][virtual]") {
    SECTION("Virtual method detection") {
        ClassTypeTestFixture fixture;
        
        auto params = makeArenaVector<const Type*>(fixture.arena);
        auto methodSig = fixture.registry.getFunctionType(params, fixture.voidType);
        
        // Create class with normal method
        auto fields1 = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methods1 = makeArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(fixture.arena);
        methods1.emplace_back(fixture.intern("normalMethod"), methodSig, nullptr);
        auto baseClass1 = fixture.noBaseClass();
        
        auto normalClass = fixture.registry.getClassType(fixture.intern("NormalClass"), 
                                                         std::move(fields1), 
                                                         std::move(methods1),
                                                         baseClass1,
                                                         flgNone, 
                                                         nullptr);
        
        // Create class with virtual method
        auto fields2 = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methods2 = makeArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(fixture.arena);
        methods2.emplace_back(fixture.intern("virtualMethod"), methodSig, nullptr);
        auto baseClass2 = fixture.noBaseClass();
        
        auto virtualClass = fixture.registry.getClassType(fixture.intern("VirtualClass"), 
                                                          std::move(fields2), 
                                                          std::move(methods2),
                                                          baseClass2,
                                                          flgVirtual,  // Mark methods as virtual
                                                          nullptr);
        
        // Create class with abstract method  
        auto fields3 = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methods3 = makeArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(fixture.arena);
        methods3.emplace_back(fixture.intern("abstractMethod"), methodSig, nullptr);
        auto baseClass3 = fixture.noBaseClass();
        
        auto abstractClass = fixture.registry.getClassType(fixture.intern("AbstractClass"), 
                                                           std::move(fields3), 
                                                           std::move(methods3),
                                                           baseClass3,
                                                           flgVirtual | flgAbstract,  // Mark methods as virtual and abstract
                                                           nullptr);
        
        // Test virtual method detection
        REQUIRE(normalClass->hasVirtualMethods() == false);
        REQUIRE(virtualClass->hasVirtualMethods() == true);
        REQUIRE(abstractClass->hasVirtualMethods() == true);
        
        // Test abstract class detection
        REQUIRE(normalClass->isAbstract() == false);
        REQUIRE(virtualClass->isAbstract() == false);
        REQUIRE(abstractClass->isAbstract() == true);
    }

    SECTION("Method override detection") {
        ClassTypeTestFixture fixture;
        
        auto params = makeArenaVector<const Type*>(fixture.arena);
        auto methodSig = fixture.registry.getFunctionType(params, fixture.voidType);
        
        // Create base class with virtual method
        auto baseFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto baseMethods = makeArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(fixture.arena);
        baseMethods.emplace_back(fixture.intern("draw"), methodSig, nullptr);
        auto baseBaseClass = fixture.noBaseClass();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("Shape"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       baseBaseClass,
                                                       flgVirtual, 
                                                       nullptr);
        
        // Create derived class that overrides method
        auto derivedFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto derivedMethods = makeArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(fixture.arena);
        derivedMethods.emplace_back(fixture.intern("draw"), methodSig, nullptr);  // Override
        auto derivedBaseClass = baseClass;
        
        auto derivedClass = fixture.registry.getClassType(fixture.intern("Circle"), 
                                                          std::move(derivedFields), 
                                                          std::move(derivedMethods),
                                                          derivedBaseClass,
                                                          flgVirtual | flgOverride,  
                                                          nullptr);
        
        // Test override detection
        REQUIRE(derivedClass->hasMethod(fixture.intern("draw")) == true);
        REQUIRE(derivedClass->hasVirtualMethods() == true);
        
        // The derived class should find its own override method, not the base method
        auto ownMethod = derivedClass->getMethod(fixture.intern("draw"), methodSig);
        auto resolvedMethod = derivedClass->resolveVirtualMethod(fixture.intern("draw"), methodSig);
        REQUIRE(ownMethod != nullptr);
        REQUIRE(resolvedMethod != nullptr);
        REQUIRE(ownMethod == resolvedMethod);  // Should find override in derived class
    }

    SECTION("Virtual method resolution") {
        ClassTypeTestFixture fixture;
        
        auto params = makeArenaVector<const Type*>(fixture.arena);
        auto methodSig = fixture.registry.getFunctionType(params, fixture.voidType);
        
        // Create base class with virtual method
        auto baseFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto baseMethods = makeArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(fixture.arena);
        baseMethods.emplace_back(fixture.intern("process"), methodSig, nullptr);
        auto baseBaseClass = fixture.noBaseClass();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("Base"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       baseBaseClass,
                                                       flgVirtual, 
                                                       nullptr);
        
        // Create derived class without override
        auto derivedFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto derivedMethods = fixture.emptyMethods();
        auto derivedBaseClass = baseClass;
        
        auto derivedClass = fixture.registry.getClassType(fixture.intern("Derived"), 
                                                          std::move(derivedFields), 
                                                          std::move(derivedMethods),
                                                          derivedBaseClass,
                                                          flgNone, 
                                                          nullptr);
        
        // Test virtual method resolution - should find base class method
        auto resolvedMethod = derivedClass->resolveVirtualMethod(fixture.intern("process"), methodSig);
        REQUIRE(resolvedMethod != nullptr);
        REQUIRE(resolvedMethod->name.view() == "process");
    }

    SECTION("Abstract class inheritance") {
        ClassTypeTestFixture fixture;
        
        auto params = makeArenaVector<const Type*>(fixture.arena);
        auto methodSig = fixture.registry.getFunctionType(params, fixture.voidType);
        
        // Create abstract base class
        auto baseFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto baseMethods = makeArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(fixture.arena);
        baseMethods.emplace_back(fixture.intern("render"), methodSig, nullptr);
        auto baseBaseClass = fixture.noBaseClass();
        
        auto abstractBase = fixture.registry.getClassType(fixture.intern("Drawable"), 
                                                          std::move(baseFields), 
                                                          std::move(baseMethods),
                                                          baseBaseClass,
                                                          flgVirtual | flgAbstract, 
                                                          nullptr);
        
        // Create concrete derived class that implements abstract method
        auto concreteFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto concreteMethods = makeArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(fixture.arena);
        concreteMethods.emplace_back(fixture.intern("render"), methodSig, nullptr);  // Implementation
        auto concreteBaseClass = abstractBase;
        
        auto concreteClass = fixture.registry.getClassType(fixture.intern("Rectangle"), 
                                                           std::move(concreteFields), 
                                                           std::move(concreteMethods),
                                                           concreteBaseClass,
                                                           flgVirtual | flgOverride,  // Implementing abstract method
                                                           nullptr);
        
        // Create incomplete derived class (still abstract)
        auto incompleteFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto incompleteMethods = fixture.emptyMethods();  // No implementation
        auto incompleteBaseClass = abstractBase;
        
        auto incompleteClass = fixture.registry.getClassType(fixture.intern("PartialShape"), 
                                                             std::move(incompleteFields), 
                                                             std::move(incompleteMethods),
                                                             incompleteBaseClass,
                                                             flgNone,  // No implementation
                                                             nullptr);
        
        // Test abstractness propagation
        REQUIRE(abstractBase->isAbstract() == true);
        REQUIRE(concreteClass->isAbstract() == false);   // Implements all abstract methods
        REQUIRE(incompleteClass->isAbstract() == true);  // Still has unimplemented abstract methods
        
        // Test inheritance relationships
        REQUIRE(concreteClass->isDerivedFrom(abstractBase) == true);
        REQUIRE(incompleteClass->isDerivedFrom(abstractBase) == true);
    }

    SECTION("Multiple virtual method inheritance") {
        ClassTypeTestFixture fixture;
        
        auto params = makeArenaVector<const Type*>(fixture.arena);
        auto voidSig = fixture.registry.getFunctionType(params, fixture.voidType);
        
        auto intParams = makeArenaVector<const Type*>(fixture.arena);
        intParams.push_back(fixture.i32Type);
        auto intSig = fixture.registry.getFunctionType(intParams, fixture.voidType);
        
        // Create base class with multiple virtual methods
        auto baseFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto baseMethods = makeArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(fixture.arena);
        baseMethods.emplace_back(fixture.intern("init"), voidSig, nullptr);
        baseMethods.emplace_back(fixture.intern("process"), intSig, nullptr);
        baseMethods.emplace_back(fixture.intern("cleanup"), voidSig, nullptr);
        auto baseBaseClass = fixture.noBaseClass();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("MultiVirtual"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       baseBaseClass,
                                                       flgVirtual, 
                                                       nullptr);
        
        // All methods should be found via virtual resolution
        REQUIRE(baseClass->resolveVirtualMethod(fixture.intern("init"), voidSig) != nullptr);
        REQUIRE(baseClass->resolveVirtualMethod(fixture.intern("process"), intSig) != nullptr);
        REQUIRE(baseClass->resolveVirtualMethod(fixture.intern("cleanup"), voidSig) != nullptr);
        REQUIRE(baseClass->resolveVirtualMethod(fixture.intern("nonexistent"), voidSig) == nullptr);
    }
}

TEST_CASE("ClassType method flags and attributes", "[types][composite][class][method-flags]") {
    SECTION("Method flag inheritance") {
        ClassTypeTestFixture fixture;
        
        auto params = makeArenaVector<const Type*>(fixture.arena);
        auto methodSig = fixture.registry.getFunctionType(params, fixture.voidType);
        
        // Test different flag combinations
        auto fields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methods = makeArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(fixture.arena);
        methods.emplace_back(fixture.intern("method"), methodSig, nullptr);
        auto baseClass = fixture.noBaseClass();
        
        // Test normal class (no virtual flags)
        auto normalClass = fixture.registry.getClassType(fixture.intern("Normal"), 
                                                         fields, methods, baseClass, flgNone, nullptr);
        
        // Test virtual class
        auto virtualClass = fixture.registry.getClassType(fixture.intern("Virtual"), 
                                                          fields, methods, baseClass, flgVirtual, nullptr);
        
        // Test abstract class  
        auto abstractClass = fixture.registry.getClassType(fixture.intern("Abstract"), 
                                                           fields, methods, baseClass, flgVirtual | flgAbstract, nullptr);
        
        REQUIRE(normalClass->hasVirtualMethods() == false);
        REQUIRE(virtualClass->hasVirtualMethods() == true);
        REQUIRE(abstractClass->hasVirtualMethods() == true);
        
        REQUIRE(normalClass->isAbstract() == false);
        REQUIRE(virtualClass->isAbstract() == false);
        REQUIRE(abstractClass->isAbstract() == true);
    }
    
    SECTION("Mixed concrete and abstract methods") {
        ClassTypeTestFixture fixture;
        
        auto params = makeArenaVector<const Type*>(fixture.arena);
        auto voidSig = fixture.registry.getFunctionType(params, fixture.voidType);
        
        auto intParams = makeArenaVector<const Type*>(fixture.arena);
        intParams.push_back(fixture.i32Type);
        auto intSig = fixture.registry.getFunctionType(intParams, fixture.i32Type);
        
        // Create class with mix of concrete and abstract methods
        auto fields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methods = makeArenaVector<std::tuple<InternedString, const FunctionType*, const ast::ASTNode*>>(fixture.arena);
        methods.emplace_back(fixture.intern("concreteMethod"), voidSig, nullptr);   
        methods.emplace_back(fixture.intern("abstractMethod"), intSig, nullptr);   
        auto baseClass = fixture.noBaseClass();
        
        // Mixed class: has both concrete and abstract methods
        auto mixedClass = fixture.registry.getClassType(fixture.intern("Mixed"), 
                                                        std::move(fields), 
                                                        std::move(methods),
                                                        baseClass,
                                                        flgVirtual | flgAbstract,  // Has abstract methods
                                                        nullptr);
        
        REQUIRE(mixedClass->hasVirtualMethods() == true);
        REQUIRE(mixedClass->isAbstract() == true);  // Has at least one abstract method
        REQUIRE(mixedClass->getMethodCount() == 2);
    }
}

TEST_CASE("ClassType flattened field layout", "[types][composite][class][flattened]") {
    SECTION("Single inheritance field flattening") {
        ClassTypeTestFixture fixture;
        
        // Create base class: Base { x: i32, y: f64 }
        auto baseFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        baseFields.emplace_back(fixture.intern("x"), fixture.i32Type);
        baseFields.emplace_back(fixture.intern("y"), fixture.f64Type);
        auto baseMethods = fixture.emptyMethods();
        auto baseBaseClass = fixture.noBaseClass();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("Base"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       baseBaseClass,
                                                       flgNone, 
                                                       nullptr);
        
        // Create derived class: Derived : Base { z: bool, w: i32 }
        auto derivedFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        derivedFields.emplace_back(fixture.intern("z"), fixture.boolType);
        derivedFields.emplace_back(fixture.intern("w"), fixture.i32Type);
        auto derivedMethods = fixture.emptyMethods();
        auto derivedBaseClass = baseClass;
        
        auto derivedClass = fixture.registry.getClassType(fixture.intern("Derived"), 
                                                          std::move(derivedFields), 
                                                          std::move(derivedMethods),
                                                          derivedBaseClass,
                                                          flgNone, 
                                                          nullptr);
        
        // Test flattened field count
        REQUIRE(baseClass->getFlattenedFieldCount() == 2);
        REQUIRE(derivedClass->getFlattenedFieldCount() == 4);  // 2 from base + 2 own
        
        // Test flattened field indices for code generation
        // Layout should be: [x: i32, y: f64, z: bool, w: i32]
        REQUIRE(derivedClass->getFlattenedFieldIndex(fixture.intern("x")) == 0);  // From base
        REQUIRE(derivedClass->getFlattenedFieldIndex(fixture.intern("y")) == 1);  // From base
        REQUIRE(derivedClass->getFlattenedFieldIndex(fixture.intern("z")) == 2);  // Own field
        REQUIRE(derivedClass->getFlattenedFieldIndex(fixture.intern("w")) == 3);  // Own field
        REQUIRE(derivedClass->getFlattenedFieldIndex(fixture.intern("nonexistent")) == SIZE_MAX);
        
        // Test logical field indices (only within each class)
        REQUIRE(baseClass->getFieldIndex(fixture.intern("x")) == 0);
        REQUIRE(baseClass->getFieldIndex(fixture.intern("y")) == 1);
        REQUIRE(derivedClass->getFieldIndex(fixture.intern("z")) == 0);  // Local to derived
        REQUIRE(derivedClass->getFieldIndex(fixture.intern("w")) == 1);  // Local to derived
        REQUIRE(derivedClass->getFieldIndex(fixture.intern("x")) == SIZE_MAX);  // Not in derived directly
    }

    SECTION("Composition via embedding (alternative to multiple inheritance)") {
        ClassTypeTestFixture fixture;
        
        // Create trait-like classes that would be embedded
        auto drawableFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        drawableFields.emplace_back(fixture.intern("x"), fixture.i32Type);
        auto drawableMethods = fixture.emptyMethods();
        auto drawableBaseClass = fixture.noBaseClass();
        
        auto drawableClass = fixture.registry.getClassType(fixture.intern("Drawable"), 
                                                           std::move(drawableFields), 
                                                           std::move(drawableMethods),
                                                           drawableBaseClass,
                                                           flgNone, 
                                                           nullptr);
        
        // Create a class that embeds other types via composition
        auto compositeFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        compositeFields.emplace_back(fixture.intern("drawable"), drawableClass);  // Embed by composition
        compositeFields.emplace_back(fixture.intern("id"), fixture.i32Type);
        auto compositeMethods = fixture.emptyMethods();
        auto compositeBaseClass = fixture.noBaseClass();
        
        auto compositeClass = fixture.registry.getClassType(fixture.intern("CompositeWidget"), 
                                                            std::move(compositeFields), 
                                                            std::move(compositeMethods),
                                                            compositeBaseClass,
                                                            flgNone, 
                                                            nullptr);
        
        // Test that composition works - the composite has its own fields
        REQUIRE(compositeClass->getFlattenedFieldCount() == 2);  // drawable + id
        REQUIRE(compositeClass->getFlattenedFieldIndex(fixture.intern("drawable")) == 0);
        REQUIRE(compositeClass->getFlattenedFieldIndex(fixture.intern("id")) == 1);
        REQUIRE(compositeClass->getFieldType(fixture.intern("drawable")) == drawableClass);
    }

    SECTION("Deep inheritance field flattening") {
        ClassTypeTestFixture fixture;
        
        // Create inheritance chain: A { x: i32 } -> B { y: f64 } -> C { z: bool }
        auto fieldsA = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        fieldsA.emplace_back(fixture.intern("x"), fixture.i32Type);
        auto methodsA = fixture.emptyMethods();
        auto baseClassA = fixture.noBaseClass();
        
        auto classA = fixture.registry.getClassType(fixture.intern("A"), 
                                                    std::move(fieldsA), 
                                                    std::move(methodsA),
                                                    baseClassA,
                                                    flgNone, 
                                                    nullptr);
        
        auto fieldsB = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        fieldsB.emplace_back(fixture.intern("y"), fixture.f64Type);
        auto methodsB = fixture.emptyMethods();
        auto baseClassB = classA;
        
        auto classB = fixture.registry.getClassType(fixture.intern("B"), 
                                                    std::move(fieldsB), 
                                                    std::move(methodsB),
                                                    baseClassB,
                                                    flgNone, 
                                                    nullptr);
        
        auto fieldsC = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        fieldsC.emplace_back(fixture.intern("z"), fixture.boolType);
        auto methodsC = fixture.emptyMethods();
        auto baseClassC = classB;
        
        auto classC = fixture.registry.getClassType(fixture.intern("C"), 
                                                    std::move(fieldsC), 
                                                    std::move(methodsC),
                                                    baseClassC,
                                                    flgNone, 
                                                    nullptr);
        
        // Test progressive field counts
        REQUIRE(classA->getFlattenedFieldCount() == 1);
        REQUIRE(classB->getFlattenedFieldCount() == 2);
        REQUIRE(classC->getFlattenedFieldCount() == 3);
        
        // Test flattened field indices for code generation
        // Layout should be: [x: i32, y: f64, z: bool]
        REQUIRE(classC->getFlattenedFieldIndex(fixture.intern("x")) == 0);  // From A
        REQUIRE(classC->getFlattenedFieldIndex(fixture.intern("y")) == 1);  // From B 
        REQUIRE(classC->getFlattenedFieldIndex(fixture.intern("z")) == 2);  // From C
        
        // Test logical field indices (local to each class)
        REQUIRE(classA->getFieldIndex(fixture.intern("x")) == 0);  // Local to A
        REQUIRE(classB->getFieldIndex(fixture.intern("y")) == 0);  // Local to B
        REQUIRE(classC->getFieldIndex(fixture.intern("z")) == 0);  // Local to C
        REQUIRE(classB->getFieldIndex(fixture.intern("x")) == SIZE_MAX);  // Not in B directly
        REQUIRE(classC->getFieldIndex(fixture.intern("x")) == SIZE_MAX);  // Not in C directly
    }

    SECTION("Flattened field offset calculation") {
        ClassTypeTestFixture fixture;
        
        // Create simple inheritance for offset testing
        auto baseFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        baseFields.emplace_back(fixture.intern("x"), fixture.i32Type);    // 4 bytes
        baseFields.emplace_back(fixture.intern("y"), fixture.f64Type);    // 8 bytes
        auto baseMethods = fixture.emptyMethods();
        auto baseBaseClass = fixture.noBaseClass();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("Base"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       baseBaseClass,
                                                       flgNone, 
                                                       nullptr);
        
        auto derivedFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        derivedFields.emplace_back(fixture.intern("z"), fixture.i32Type);  // 4 bytes
        auto derivedMethods = fixture.emptyMethods();
        auto derivedBaseClass = baseClass;
        
        auto derivedClass = fixture.registry.getClassType(fixture.intern("Derived"), 
                                                          std::move(derivedFields), 
                                                          std::move(derivedMethods),
                                                          derivedBaseClass,
                                                          flgNone, 
                                                          nullptr);
        
        // Test offset calculations (these will depend on alignment, but we can test relativity)
        size_t xOffset = derivedClass->getFlattenedFieldOffset(fixture.intern("x"));
        size_t yOffset = derivedClass->getFlattenedFieldOffset(fixture.intern("y"));
        size_t zOffset = derivedClass->getFlattenedFieldOffset(fixture.intern("z"));
        
        REQUIRE(xOffset != SIZE_MAX);  // Should be found
        REQUIRE(yOffset != SIZE_MAX);  // Should be found 
        REQUIRE(zOffset != SIZE_MAX);  // Should be found
        REQUIRE(xOffset < yOffset);    // x comes before y
        REQUIRE(yOffset < zOffset);    // y comes before z
        
        // Test offset by flattened index
        REQUIRE(derivedClass->getFlattenedFieldOffset(0) == xOffset);
        REQUIRE(derivedClass->getFlattenedFieldOffset(1) == yOffset);
        REQUIRE(derivedClass->getFlattenedFieldOffset(2) == zOffset);
        REQUIRE(derivedClass->getFlattenedFieldOffset(999) == SIZE_MAX);  // Out of bounds
        
        // Test that flattened indices match our offset calculations
        REQUIRE(derivedClass->getFlattenedFieldIndex(fixture.intern("x")) == 0);
        REQUIRE(derivedClass->getFlattenedFieldIndex(fixture.intern("y")) == 1);
        REQUIRE(derivedClass->getFlattenedFieldIndex(fixture.intern("z")) == 2);
    }

    SECTION("Logical vs flattened field index distinction") {
        ClassTypeTestFixture fixture;
        
        // Create base class: Base { x: i32, y: f64 }
        auto baseFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        baseFields.emplace_back(fixture.intern("x"), fixture.i32Type);
        baseFields.emplace_back(fixture.intern("y"), fixture.f64Type);
        auto baseMethods = fixture.emptyMethods();
        auto baseBaseClass = fixture.noBaseClass();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("Base"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       baseBaseClass,
                                                       flgNone, 
                                                       nullptr);
        
        // Create derived class: Derived : Base { a: i32, b: bool }
        auto derivedFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        derivedFields.emplace_back(fixture.intern("a"), fixture.i32Type);
        derivedFields.emplace_back(fixture.intern("b"), fixture.boolType);
        auto derivedMethods = fixture.emptyMethods();
        auto derivedBaseClass = baseClass;
        
        auto derivedClass = fixture.registry.getClassType(fixture.intern("Derived"), 
                                                          std::move(derivedFields), 
                                                          std::move(derivedMethods),
                                                          derivedBaseClass,
                                                          flgNone, 
                                                          nullptr);
        
        // LOGICAL INDICES (within each class)
        // Base class fields
        REQUIRE(baseClass->getFieldIndex(fixture.intern("x")) == 0);
        REQUIRE(baseClass->getFieldIndex(fixture.intern("y")) == 1);
        
        // Derived class only knows about its own fields for logical indexing
        REQUIRE(derivedClass->getFieldIndex(fixture.intern("a")) == 0);  // Local field 0
        REQUIRE(derivedClass->getFieldIndex(fixture.intern("b")) == 1);  // Local field 1
        REQUIRE(derivedClass->getFieldIndex(fixture.intern("x")) == SIZE_MAX);  // Not local field
        REQUIRE(derivedClass->getFieldIndex(fixture.intern("y")) == SIZE_MAX);  // Not local field
        
        // FLATTENED INDICES (for code generation)
        // All fields in inheritance order: [x, y, a, b]
        REQUIRE(derivedClass->getFlattenedFieldIndex(fixture.intern("x")) == 0);  // Base field
        REQUIRE(derivedClass->getFlattenedFieldIndex(fixture.intern("y")) == 1);  // Base field
        REQUIRE(derivedClass->getFlattenedFieldIndex(fixture.intern("a")) == 2);  // Derived field
        REQUIRE(derivedClass->getFlattenedFieldIndex(fixture.intern("b")) == 3);  // Derived field
        
        // BUT derived class still sees inherited fields via hasField/getFieldType
        REQUIRE(derivedClass->hasField(fixture.intern("x")) == true);    // Inherited
        REQUIRE(derivedClass->hasField(fixture.intern("y")) == true);    // Inherited
        REQUIRE(derivedClass->hasField(fixture.intern("a")) == true);    // Own
        REQUIRE(derivedClass->hasField(fixture.intern("b")) == true);    // Own
        REQUIRE(derivedClass->getFieldType(fixture.intern("x")) == fixture.i32Type);
        REQUIRE(derivedClass->getFieldType(fixture.intern("y")) == fixture.f64Type);
    }
}

TEST_CASE("ClassType equality and type relationships", "[types][composite][class][equality]") {
    SECTION("ClassType equality") {
        ClassTypeTestFixture fixture;
        
        auto fields1 = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        fields1.emplace_back(fixture.intern("x"), fixture.i32Type);
        
        auto methods1 = fixture.emptyMethods();
        auto baseClass1 = fixture.noBaseClass();
        
        auto class1 = fixture.registry.getClassType(fixture.intern("Point"), 
                                                    std::move(fields1), 
                                                    std::move(methods1),
                                                    baseClass1,
                                                    flgNone, 
                                                    nullptr);
        
        auto fields2 = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        fields2.emplace_back(fixture.intern("x"), fixture.i32Type);
        
        auto methods2 = fixture.emptyMethods();
        auto baseClass2 = fixture.noBaseClass();
        
        auto class2 = fixture.registry.getClassType(fixture.intern("Point"), 
                                                    std::move(fields2), 
                                                    std::move(methods2),
                                                    baseClass2,
                                                    flgNone, 
                                                    nullptr);
        
        // Same definition should return cached instance
        REQUIRE(class1 == class2);
        REQUIRE(class1->equals(class2) == true);
    }

    SECTION("ClassType compatibility") {
        ClassTypeTestFixture fixture;
        
        // Create two different classes
        auto fields1 = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methods1 = fixture.emptyMethods();
        auto baseClass1 = fixture.noBaseClass();
        
        auto class1 = fixture.registry.getClassType(fixture.intern("ClassA"), 
                                                    std::move(fields1), 
                                                    std::move(methods1),
                                                    baseClass1,
                                                    flgNone, 
                                                    nullptr);
        
        auto fields2 = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methods2 = fixture.emptyMethods();
        auto baseClass2 = fixture.noBaseClass();
        
        auto class2 = fixture.registry.getClassType(fixture.intern("ClassB"), 
                                                    std::move(fields2), 
                                                    std::move(methods2),
                                                    baseClass2,
                                                    flgNone, 
                                                    nullptr);
        
        // Different classes should not be assignable
        REQUIRE(class1->equals(class2) == false);
        REQUIRE(class1->isAssignableFrom(class2) == false);
        REQUIRE(class1->isCompatibleWith(class2) == false);
    }
}

TEST_CASE("ClassType inheritance-based assignment and conversion", "[types][composite][class][inheritance][assignment]") {
    SECTION("Assignment compatibility with inheritance") {
        ClassTypeTestFixture fixture;
        
        // Create base class
        auto baseFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        baseFields.emplace_back(fixture.intern("id"), fixture.i32Type);
        
        auto baseMethods = fixture.emptyMethods();
        auto baseBaseClass = fixture.noBaseClass();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("Base"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       baseBaseClass,
                                                       flgNone, 
                                                       nullptr);
        
        // Create derived class
        auto derivedFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        derivedFields.emplace_back(fixture.intern("value"), fixture.f64Type);
        
        auto derivedMethods = fixture.emptyMethods();
        auto derivedBaseClass = baseClass;
        
        auto derivedClass = fixture.registry.getClassType(fixture.intern("Derived"), 
                                                          std::move(derivedFields), 
                                                          std::move(derivedMethods),
                                                          derivedBaseClass,
                                                          flgNone, 
                                                          nullptr);
        
        // Test assignment compatibility
        // Base should be assignable from derived (polymorphic assignment)
        REQUIRE(baseClass->isAssignableFrom(derivedClass) == true);
        
        // Derived should NOT be assignable from base (unsafe downcast)
        REQUIRE(derivedClass->isAssignableFrom(baseClass) == false);
        
        // Both should be assignable from themselves (identity)
        REQUIRE(baseClass->isAssignableFrom(baseClass) == true);
        REQUIRE(derivedClass->isAssignableFrom(derivedClass) == true);
    }

    SECTION("Implicit conversion with inheritance") {
        ClassTypeTestFixture fixture;
        
        // Create inheritance chain: A -> B -> C
        auto fieldsA = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methodsA = fixture.emptyMethods();
        auto baseClassA = fixture.noBaseClass();
        
        auto classA = fixture.registry.getClassType(fixture.intern("A"), 
                                                    std::move(fieldsA), 
                                                    std::move(methodsA),
                                                    baseClassA,
                                                    flgNone, 
                                                    nullptr);
        
        auto fieldsB = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methodsB = fixture.emptyMethods();
        auto baseClassB = classA;
        
        auto classB = fixture.registry.getClassType(fixture.intern("B"), 
                                                    std::move(fieldsB), 
                                                    std::move(methodsB),
                                                    baseClassB,
                                                    flgNone, 
                                                    nullptr);
        
        auto fieldsC = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methodsC = fixture.emptyMethods();
        auto baseClassC = classB;
        
        auto classC = fixture.registry.getClassType(fixture.intern("C"), 
                                                    std::move(fieldsC), 
                                                    std::move(methodsC),
                                                    baseClassC,
                                                    flgNone, 
                                                    nullptr);
        
        // Test implicit upcasting (safe)
        REQUIRE(classB->isImplicitlyConvertibleTo(classA) == true);   // B -> A
        REQUIRE(classC->isImplicitlyConvertibleTo(classA) == true);   // C -> A (transitive)
        REQUIRE(classC->isImplicitlyConvertibleTo(classB) == true);   // C -> B
        
        // Test implicit downcasting (should be forbidden)
        REQUIRE(classA->isImplicitlyConvertibleTo(classB) == false);  // A -> B (unsafe)
        REQUIRE(classA->isImplicitlyConvertibleTo(classC) == false);  // A -> C (unsafe)
        REQUIRE(classB->isImplicitlyConvertibleTo(classC) == false);  // B -> C (unsafe)
        
        // Test implicit conversion to self (identity)
        REQUIRE(classA->isImplicitlyConvertibleTo(classA) == true);   // Self-conversion via equals()
        REQUIRE(classB->isImplicitlyConvertibleTo(classB) == true);   // Self-conversion via equals()
        REQUIRE(classC->isImplicitlyConvertibleTo(classC) == true);   // Self-conversion via equals()
    }

    SECTION("Explicit conversion with inheritance") {
        ClassTypeTestFixture fixture;
        
        // Create simple inheritance: Base -> Derived
        auto baseFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto baseMethods = fixture.emptyMethods();
        auto baseBaseClass = fixture.noBaseClass();
        
        auto baseClass = fixture.registry.getClassType(fixture.intern("Base"), 
                                                       std::move(baseFields), 
                                                       std::move(baseMethods),
                                                       baseBaseClass,
                                                       flgNone, 
                                                       nullptr);
        
        auto derivedFields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto derivedMethods = fixture.emptyMethods();
        auto derivedBaseClass = baseClass;
        
        auto derivedClass = fixture.registry.getClassType(fixture.intern("Derived"), 
                                                          std::move(derivedFields), 
                                                          std::move(derivedMethods),
                                                          derivedBaseClass,
                                                          flgNone, 
                                                          nullptr);
        
        // Test explicit upcasting (safe, should be allowed)
        REQUIRE(derivedClass->isExplicitlyConvertibleTo(baseClass) == true);
        
        // Test explicit downcasting (potentially unsafe, but allowed with explicit cast)
        REQUIRE(baseClass->isExplicitlyConvertibleTo(derivedClass) == true);
        
        // Test explicit conversion to self (should work through equals)
        REQUIRE(baseClass->isExplicitlyConvertibleTo(baseClass) == true);    // equals() path
        REQUIRE(derivedClass->isExplicitlyConvertibleTo(derivedClass) == true); // equals() path
    }

    SECTION("Conversion with unrelated classes") {
        ClassTypeTestFixture fixture;
        
        // Create two unrelated classes
        auto fields1 = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methods1 = fixture.emptyMethods();
        auto baseClass1 = fixture.noBaseClass();
        
        auto class1 = fixture.registry.getClassType(fixture.intern("Class1"), 
                                                    std::move(fields1), 
                                                    std::move(methods1),
                                                    baseClass1,
                                                    flgNone, 
                                                    nullptr);
        
        auto fields2 = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methods2 = fixture.emptyMethods();
        auto baseClass2 = fixture.noBaseClass();
        
        auto class2 = fixture.registry.getClassType(fixture.intern("Class2"), 
                                                    std::move(fields2), 
                                                    std::move(methods2),
                                                    baseClass2,
                                                    flgNone, 
                                                    nullptr);
        
        // Unrelated classes should not be assignable or convertible
        REQUIRE(class1->isAssignableFrom(class2) == false);
        REQUIRE(class2->isAssignableFrom(class1) == false);
        
        REQUIRE(class1->isImplicitlyConvertibleTo(class2) == false);
        REQUIRE(class2->isImplicitlyConvertibleTo(class1) == false);
        
        REQUIRE(class1->isExplicitlyConvertibleTo(class2) == false);
        REQUIRE(class2->isExplicitlyConvertibleTo(class1) == false);
    }
}

TEST_CASE("ClassType registry caching", "[types][composite][class][registry]") {
    SECTION("ClassType caching works correctly") {
        ClassTypeTestFixture fixture;
        
        auto fields1 = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        fields1.emplace_back(fixture.intern("value"), fixture.i32Type);
        
        auto methods1 = fixture.emptyMethods();
        auto baseClass1 = fixture.noBaseClass();
        
        auto class1 = fixture.registry.getClassType(fixture.intern("CachedClass"), 
                                                    std::move(fields1), 
                                                    std::move(methods1),
                                                    baseClass1,
                                                    flgNone, 
                                                    nullptr);
        
        auto fields2 = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        fields2.emplace_back(fixture.intern("value"), fixture.i32Type);
        
        auto methods2 = fixture.emptyMethods();
        auto baseClass2 = fixture.noBaseClass();
        
        auto class2 = fixture.registry.getClassType(fixture.intern("CachedClass"), 
                                                    std::move(fields2), 
                                                    std::move(methods2),
                                                    baseClass2,
                                                    flgNone, 
                                                    nullptr);
        
        // Should return the same cached instance
        REQUIRE(class1 == class2);
    }

    SECTION("Type count includes class types") {
        ClassTypeTestFixture fixture;
        auto beforeCount = fixture.registry.getTypeCount();
        
        auto fields = makeArenaVector<std::pair<InternedString, const Type*>>(fixture.arena);
        auto methods = fixture.emptyMethods();
        auto baseClass = fixture.noBaseClass();
        
        auto classType = fixture.registry.getClassType(fixture.intern("CountTest"), 
                                                       std::move(fields), 
                                                       std::move(methods),
                                                       baseClass,
                                                       flgNone, 
                                                       nullptr);
        
        auto afterCount = fixture.registry.getTypeCount();
        
        REQUIRE(afterCount > beforeCount);
    }
}