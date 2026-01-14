#include "catch2.hpp"
#include "cxy/token.hpp"
#include "cxy/diagnostics.hpp"
#include "cxy/strings.hpp"
#include "cxy/memory.hpp"

using namespace cxy;
using Catch::Approx;

TEST_CASE("Token basic construction", "[token]") {
    Location location{"test.cxy", Position{1, 1, 0}, Position{1, 5, 4}};
    
    SECTION("Basic construction without values") {
        Token token(TokenKind::LParen, location);
        
        REQUIRE(token.kind == TokenKind::LParen);
        REQUIRE(token.location == location);
        REQUIRE_FALSE(token.hasLiteralValue());
        REQUIRE(token.isValid());
        REQUIRE_FALSE(token.isEof());
    }
    
    SECTION("Default construction") {
        Token token;
        
        REQUIRE(token.kind == TokenKind::Error);
        REQUIRE_FALSE(token.hasLiteralValue());
        REQUIRE_FALSE(token.isValid());
    }
}

TEST_CASE("Token boolean values", "[token]") {
    Location location{"test.cxy", Position{1, 1, 0}, Position{1, 5, 4}};
    
    Token trueToken(TokenKind::True, location, true);
    Token falseToken(TokenKind::False, location, false);
    
    SECTION("True token") {
        REQUIRE(trueToken.kind == TokenKind::True);
        REQUIRE(trueToken.hasLiteralValue());
        REQUIRE(trueToken.getBoolValue());
    }
    
    SECTION("False token") {
        REQUIRE(falseToken.kind == TokenKind::False);
        REQUIRE(falseToken.hasLiteralValue());
        REQUIRE_FALSE(falseToken.getBoolValue());
    }
    
    SECTION("Equality") {
        Token anotherTrue(TokenKind::True, location, true);
        REQUIRE(trueToken == anotherTrue);
        REQUIRE_FALSE(trueToken == falseToken);
    }
}

TEST_CASE("Token character values", "[token]") {
    Location location{"test.cxy", Position{1, 1, 0}, Position{1, 5, 4}};
    
    Token charToken(TokenKind::CharLiteral, location, static_cast<uint32_t>(0x41)); // 'A'
    Token emojiToken(TokenKind::CharLiteral, location, static_cast<uint32_t>(0x1F600)); // 😀
    
    SECTION("Basic character") {
        REQUIRE(charToken.kind == TokenKind::CharLiteral);
        REQUIRE(charToken.hasLiteralValue());
        REQUIRE(charToken.getCharValue() == 0x41u);
    }
    
    SECTION("Unicode emoji") {
        REQUIRE(emojiToken.kind == TokenKind::CharLiteral);
        REQUIRE(emojiToken.hasLiteralValue());
        REQUIRE(emojiToken.getCharValue() == 0x1F600u);
    }
    
    SECTION("Equality") {
        Token anotherChar(TokenKind::CharLiteral, location, static_cast<uint32_t>(0x41));
        REQUIRE(charToken == anotherChar);
        REQUIRE_FALSE(charToken == emojiToken);
    }
}

TEST_CASE("Token integer values", "[token]") {
    Location location{"test.cxy", Position{1, 1, 0}, Position{1, 5, 0}};
    
    Token smallInt(TokenKind::IntLiteral, location, static_cast<__uint128_t>(42));
    Token largeInt(TokenKind::IntLiteral, location, static_cast<__uint128_t>(0xFFFFFFFFFFFFFFFFULL));
    
    SECTION("Small integer") {
        REQUIRE(smallInt.kind == TokenKind::IntLiteral);
        REQUIRE(smallInt.hasLiteralValue());
        REQUIRE(smallInt.getIntValue() == 42u);
    }
    
    SECTION("Large integer") {
        REQUIRE(largeInt.kind == TokenKind::IntLiteral);
        REQUIRE(largeInt.hasLiteralValue());
        REQUIRE(largeInt.getIntValue() == 0xFFFFFFFFFFFFFFFFULL);
    }
    
    SECTION("Equality") {
        Token anotherSmall(TokenKind::IntLiteral, location, static_cast<__uint128_t>(42));
        REQUIRE(smallInt == anotherSmall);
        REQUIRE_FALSE(smallInt == largeInt);
    }
}

TEST_CASE("Token floating-point values", "[token]") {
    Location location{"test.cxy", Position{1, 1, 0}, Position{1, 5, 0}};
    
    Token piToken(TokenKind::FloatLiteral, location, 3.14159);
    Token eToken(TokenKind::FloatLiteral, location, 2.71828);
    
    SECTION("Pi token") {
        REQUIRE(piToken.kind == TokenKind::FloatLiteral);
        REQUIRE(piToken.hasLiteralValue());
        REQUIRE(piToken.getFloatValue() == Approx(3.14159));
    }
    
    SECTION("E token") {
        REQUIRE(eToken.kind == TokenKind::FloatLiteral);
        REQUIRE(eToken.hasLiteralValue());
        REQUIRE(eToken.getFloatValue() == Approx(2.71828));
    }
    
    SECTION("Equality") {
        Token anotherPi(TokenKind::FloatLiteral, location, 3.14159);
        REQUIRE(piToken == anotherPi);
        REQUIRE_FALSE(piToken == eToken);
    }
}

TEST_CASE("Token getter defaults", "[token]") {
    Location location{"test.cxy", Position{1, 1, 0}, Position{1, 5, 0}};
    Token identToken(TokenKind::Ident, location);
    
    // Should return defaults since this isn't a literal token
    REQUIRE_FALSE(identToken.hasLiteralValue());
    REQUIRE_FALSE(identToken.getBoolValue());
    REQUIRE(identToken.getCharValue() == 0u);
    REQUIRE(identToken.getIntValue() == 0u);
    REQUIRE(identToken.getFloatValue() == Approx(0.0));
}

TEST_CASE("Token getter type validation", "[token]") {
    Location location{"test.cxy", Position{1, 1, 0}, Position{1, 5, 0}};
    Token intToken(TokenKind::IntLiteral, location, static_cast<__uint128_t>(42));
    
    // Should return default for wrong getter types
    REQUIRE_FALSE(intToken.getBoolValue());
    REQUIRE(intToken.getCharValue() == 0u);
    REQUIRE(intToken.getFloatValue() == Approx(0.0));
    
    // Should return correct value for right getter
    REQUIRE(intToken.getIntValue() == 42u);
}

TEST_CASE("Token isOneOf functionality", "[token]") {
    Location location{"test.cxy", Position{1, 1, 0}, Position{1, 5, 0}};
    
    Token plusToken(TokenKind::Plus, location);
    Token minusToken(TokenKind::Minus, location);
    Token identToken(TokenKind::Ident, location);
    
    SECTION("Two options") {
        REQUIRE(plusToken.isOneOf(TokenKind::Plus, TokenKind::Minus));
        REQUIRE(minusToken.isOneOf(TokenKind::Plus, TokenKind::Minus));
        REQUIRE_FALSE(identToken.isOneOf(TokenKind::Plus, TokenKind::Minus));
    }
    
    SECTION("Single argument") {
        REQUIRE(plusToken.isOneOf(TokenKind::Plus));
        REQUIRE_FALSE(plusToken.isOneOf(TokenKind::Minus));
    }
    
    SECTION("Multiple arguments") {
        REQUIRE(plusToken.isOneOf(TokenKind::Plus, TokenKind::Minus, TokenKind::Mult));
        REQUIRE_FALSE(identToken.isOneOf(TokenKind::Plus, TokenKind::Minus, TokenKind::Mult));
    }
}

TEST_CASE("Token EOF handling", "[token]") {
    Location location{"test.cxy", Position{1, 1, 0}, Position{1, 5, 0}};
    Token eofToken(TokenKind::EoF, location);
    
    REQUIRE(eofToken.isEof());
    REQUIRE(eofToken.isValid()); // EoF is considered valid
    REQUIRE_FALSE(eofToken.hasLiteralValue());
}

TEST_CASE("Token large integers", "[token]") {
    Location location{"test.cxy", Position{1, 1, 0}, Position{1, 5, 0}};
    
    // Test maximum 128-bit value
    __uint128_t maxVal = ~static_cast<__uint128_t>(0);
    Token maxToken(TokenKind::IntLiteral, location, maxVal);
    
    REQUIRE(maxToken.hasLiteralValue());
    REQUIRE(maxToken.getIntValue() == maxVal);
}

TEST_CASE("Token special float values", "[token]") {
    Location location{"test.cxy", Position{1, 1, 0}, Position{1, 5, 0}};
    
    Token infToken(TokenKind::FloatLiteral, location, std::numeric_limits<double>::infinity());
    Token nanToken(TokenKind::FloatLiteral, location, std::numeric_limits<double>::quiet_NaN());
    Token zeroToken(TokenKind::FloatLiteral, location, 0.0);
    Token negZeroToken(TokenKind::FloatLiteral, location, -0.0);
    
    REQUIRE(std::isinf(infToken.getFloatValue()));
    REQUIRE(std::isnan(nanToken.getFloatValue()));
    REQUIRE(zeroToken.getFloatValue() == Approx(0.0));
    REQUIRE(negZeroToken.getFloatValue() == Approx(-0.0));
}

TEST_CASE("Token equality with different locations", "[token]") {
    Location location1{"test.cxy", Position{1, 1, 0}, Position{1, 5, 0}};
    Location location2{"test.cxy", Position{2, 1, 0}, Position{2, 5, 0}};
    
    Token token1(TokenKind::IntLiteral, location1, static_cast<__uint128_t>(42));
    Token token2(TokenKind::IntLiteral, location2, static_cast<__uint128_t>(42));
    
    // Should not be equal due to different locations
    REQUIRE_FALSE(token1 == token2);
}

TEST_CASE("Token inequality operator", "[token]") {
    Location location{"test.cxy", Position{1, 1, 0}, Position{1, 5, 0}};
    
    Token token1(TokenKind::True, location, true);
    Token token2(TokenKind::False, location, false);
    Token token3(TokenKind::True, location, true);
    
    REQUIRE(token1 != token2);
    REQUIRE_FALSE(token1 != token3);
}

// ===== TOKEN READING TESTS =====

TEST_CASE("Token reading from source", "[token][reading]") {
    // Setup source manager with test content
    SourceManager sourceManager;
    ArenaAllocator arena(4096);
    StringInterner interner(arena);
    
    std::string testSource = "func main() {\n"
                           "    var x = 42;\n"
                           "    var name = \"hello\";\n"
                           "    var flag = true;\n"
                           "}";
    
    sourceManager.registerFile("test.cxy", testSource);
    
    SECTION("Reading fixed token text (symbols and keywords)") {
        // Test keyword - "func" spans bytes 0-3 (4 characters)
        Location funcLoc("test.cxy", Position{1, 1, 0}, Position{1, 5, 4});
        Token funcToken(TokenKind::Func, funcLoc);
        
        std::string_view text = readTokenText(funcToken, sourceManager);
        REQUIRE(text == "func");
        
        // Test symbol - "(" at byte 10 (1 character)
        Location parenLoc("test.cxy", Position{1, 11, 10}, Position{1, 12, 11});
        Token parenToken(TokenKind::LParen, parenLoc);
        
        std::string_view parenText = readTokenText(parenToken, sourceManager);
        REQUIRE(parenText == "(");
    }
    
    SECTION("Reading variable token text from source") {
        // Test identifier "main" spans bytes 5-8 (4 characters)
        Location mainLoc("test.cxy", Position{1, 6, 5}, Position{1, 10, 9});
        Token mainToken(TokenKind::Ident, mainLoc);
        
        std::string_view text = readTokenText(mainToken, sourceManager);
        REQUIRE(text == "main");
        
        // Test identifier "x" - it's at byte 22, end exclusive
        Location xLoc("test.cxy", Position{2, 9, 22}, Position{2, 10, 23});
        Token xToken(TokenKind::Ident, xLoc);
        
        std::string_view xText = readTokenText(xToken, sourceManager);
        REQUIRE(xText == "x");
    }
    
    SECTION("Reading integer literal") {
        // Test "42" - it spans bytes 26-27, end exclusive at 28
        Location intLoc("test.cxy", Position{2, 13, 26}, Position{2, 15, 28});
        Token intToken(TokenKind::IntLiteral, intLoc);
        
        std::string_view intText = readTokenText(intToken, sourceManager);
        REQUIRE(intText == "42");
    }
    
    SECTION("Reading string literal") {
        // Test "\"hello\"" - it spans bytes 45-51, end exclusive
        Location stringLoc("test.cxy", Position{3, 16, 45}, Position{3, 23, 52});
        Token stringToken(TokenKind::StringLiteral, stringLoc);
        
        std::string_view stringText = readTokenText(stringToken, sourceManager);
        REQUIRE(stringText == "\"hello\"");
    }
    
    SECTION("Reading boolean literal") {
        // Test "true" - it spans bytes 69-72, end exclusive
        Location boolLoc("test.cxy", Position{4, 16, 69}, Position{4, 20, 73});
        Token boolToken(TokenKind::True, boolLoc);
        
        // For boolean literals, should return the keyword text
        std::string_view boolText = readTokenText(boolToken, sourceManager);
        REQUIRE(boolText == "true");
    }
}

TEST_CASE("Token interning functionality", "[token][interning]") {
    SourceManager sourceManager;
    ArenaAllocator arena(4096);
    StringInterner interner(arena);
    
    std::string testSource = "var main = func() { return main; }";
    sourceManager.registerFile("test.cxy", testSource);
    
    SECTION("shouldInternTokenText validation") {
        // Should intern identifiers
        REQUIRE(shouldInternTokenText(TokenKind::Ident));
        
        // Should intern keywords
        REQUIRE(shouldInternTokenText(TokenKind::Func));
        REQUIRE(shouldInternTokenText(TokenKind::Var));
        REQUIRE(shouldInternTokenText(TokenKind::If));
        
        // Should intern string literals
        REQUIRE(shouldInternTokenText(TokenKind::StringLiteral));
        
        // Should not intern symbols
        REQUIRE_FALSE(shouldInternTokenText(TokenKind::LParen));
        REQUIRE_FALSE(shouldInternTokenText(TokenKind::Plus));
        REQUIRE_FALSE(shouldInternTokenText(TokenKind::Equal));
        
        // Should not intern other literals
        REQUIRE_FALSE(shouldInternTokenText(TokenKind::IntLiteral));
        REQUIRE_FALSE(shouldInternTokenText(TokenKind::FloatLiteral));
        REQUIRE_FALSE(shouldInternTokenText(TokenKind::CharLiteral));
        
        // Should not intern special tokens
        REQUIRE_FALSE(shouldInternTokenText(TokenKind::EoF));
        REQUIRE_FALSE(shouldInternTokenText(TokenKind::Error));
    }
    
    SECTION("getTokenValue with interning") {
        // Test identifier "main" - spans bytes 4-7, end exclusive at 8  
        Location mainLoc("test.cxy", Position{1, 5, 4}, Position{1, 9, 8});
        Token mainToken(TokenKind::Ident, mainLoc);
        
        InternedString internedMain = getTokenValue(mainToken, sourceManager, interner);
        std::string_view mainText(internedMain.c_str(), internedMain.size());
        REQUIRE(mainText == "main");
        
        // Second "main" at end: spans bytes 27-30, end exclusive
        Location anotherMainLoc("test.cxy", Position{1, 28, 27}, Position{1, 32, 31});
        Token anotherMainToken(TokenKind::Ident, anotherMainLoc);
        
        InternedString anotherInternedMain = getTokenValue(anotherMainToken, sourceManager, interner);
        REQUIRE(internedMain == anotherInternedMain);
        
        // Test keyword interning - "func" spans bytes 11-14
        Location funcLoc("test.cxy", Position{1, 12, 11}, Position{1, 16, 15});
        Token funcToken(TokenKind::Func, funcLoc);
        
        InternedString internedFunc = getTokenValue(funcToken, sourceManager, interner);
        std::string_view funcText(internedFunc.c_str(), internedFunc.size());
        REQUIRE(funcText == "func");
    }
    
    SECTION("getTokenText without interning") {
        Location mainLoc("test.cxy", Position{1, 5, 4}, Position{1, 9, 8});
        Token mainToken(TokenKind::Ident, mainLoc);
        
        std::string_view text = getTokenText(mainToken, sourceManager);
        REQUIRE(text == "main");
        
        // Test symbol - "(" at byte 9
        Location parenLoc("test.cxy", Position{1, 10, 9}, Position{1, 11, 10});
        Token parenToken(TokenKind::LParen, parenLoc);
        
        std::string_view parenText = getTokenText(parenToken, sourceManager);
        REQUIRE(parenText == "(");
    }
}

TEST_CASE("Token edge cases", "[token][edge_cases]") {
    SourceManager sourceManager;
    ArenaAllocator arena(4096);
    StringInterner interner(arena);
    
    SECTION("EOF token reading") {
        std::string testSource = "func main() {}";
        sourceManager.registerFile("test.cxy", testSource);
        
        Location eofLoc("test.cxy", Position{1, 15, 14}, Position{1, 15, 14});
        Token eofToken(TokenKind::EoF, eofLoc);
        
        std::string_view text = readTokenText(eofToken, sourceManager);
        REQUIRE(text == "");
    }
    
    SECTION("Error token reading") {
        std::string testSource = "func main() {}";
        sourceManager.registerFile("test.cxy", testSource);
        
        Location errorLoc("test.cxy", Position{1, 1, 0}, Position{1, 5, 4});
        Token errorToken(TokenKind::Error, errorLoc);
        
        // Should try to read from source even for error tokens
        std::string_view text = readTokenText(errorToken, sourceManager);
        REQUIRE(text == "func");
    }
    
    SECTION("Empty source handling") {
        sourceManager.registerFile("empty.cxy", "");
        
        Location emptyLoc("empty.cxy", Position{1, 1, 0}, Position{1, 1, 0});
        Token emptyToken(TokenKind::Ident, emptyLoc);
        
        std::string_view text = readTokenText(emptyToken, sourceManager);
        REQUIRE(text == "");
    }
    
    SECTION("Interning efficiency test") {
        std::string testSource = "var x = x + x;";
        sourceManager.registerFile("efficiency.cxy", testSource);
        
        // Create multiple tokens with the same text "x"
        // "var " = 4 bytes, first "x" at byte 4
        Location loc1("efficiency.cxy", Position{1, 5, 4}, Position{1, 6, 5});
        // "var x = " = 8 bytes, second "x" at byte 8 
        Location loc2("efficiency.cxy", Position{1, 9, 8}, Position{1, 10, 9});
        // "var x = x + " = 12 bytes, third "x" at byte 12
        Location loc3("efficiency.cxy", Position{1, 13, 12}, Position{1, 14, 13});
        
        Token token1(TokenKind::Ident, loc1);
        Token token2(TokenKind::Ident, loc2);
        Token token3(TokenKind::Ident, loc3);
        
        InternedString handle1 = getTokenValue(token1, sourceManager, interner);
        InternedString handle2 = getTokenValue(token2, sourceManager, interner);
        InternedString handle3 = getTokenValue(token3, sourceManager, interner);
        
        // Should get the same interned string handle for all three
        REQUIRE(handle1 == handle2);
        REQUIRE(handle2 == handle3);
        
        // Verify they all point to the same string
        std::string_view text1(handle1.c_str(), handle1.size());
        std::string_view text2(handle2.c_str(), handle2.size());
        std::string_view text3(handle3.c_str(), handle3.size());
        
        REQUIRE(text1 == "x");
        REQUIRE(text2 == "x");
        REQUIRE(text3 == "x");
        
        // They should actually be the same memory location due to interning
        REQUIRE(text1.data() == text2.data());
        REQUIRE(text2.data() == text3.data());
    }
}