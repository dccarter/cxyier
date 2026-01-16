#include "catch2.hpp"
#include "cxy/diagnostics.hpp"
#include "cxy/lexer.hpp"
#include "lexer_test_helper.hpp"

#include <iostream>
#include <memory>

namespace cxy {

// Using LexerTestHelper from lexer_test_helper.hpp

// Test basic punctuation and operators
TEST_CASE("Lexer can tokenize basic operators", "[lexer][phase1]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("+ - * / = == != < > <= >=");

  REQUIRE(tokens.size() == 12); // 11 operators + EOF
  CHECK(tokens[0].kind == TokenKind::Plus);
  CHECK(tokens[1].kind == TokenKind::Minus);
  CHECK(tokens[2].kind == TokenKind::Mult);
  CHECK(tokens[3].kind == TokenKind::Div);
  CHECK(tokens[4].kind == TokenKind::Assign);
  CHECK(tokens[5].kind == TokenKind::Equal);
  CHECK(tokens[6].kind == TokenKind::NotEqual);
  CHECK(tokens[7].kind == TokenKind::Less);
  CHECK(tokens[8].kind == TokenKind::Greater);
  CHECK(tokens[9].kind == TokenKind::LessEqual);
  CHECK(tokens[10].kind == TokenKind::GreaterEqual);
  CHECK(tokens[11].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can tokenize basic punctuation", "[lexer][phase1]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("; , . ( ) { } [ ]");

  REQUIRE(tokens.size() == 10); // 9 punctuation + EOF
  CHECK(tokens[0].kind == TokenKind::Semicolon);
  CHECK(tokens[1].kind == TokenKind::Comma);
  CHECK(tokens[2].kind == TokenKind::Dot);
  CHECK(tokens[3].kind == TokenKind::LParen);
  CHECK(tokens[4].kind == TokenKind::RParen);
  CHECK(tokens[5].kind == TokenKind::LBrace);
  CHECK(tokens[6].kind == TokenKind::RBrace);
  CHECK(tokens[7].kind == TokenKind::LBracket);
  CHECK(tokens[8].kind == TokenKind::RBracket);
  CHECK(tokens[9].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can tokenize basic identifiers", "[lexer][phase1]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("hello world foo42 _test variable_name");

  REQUIRE(tokens.size() == 6); // 5 identifiers + EOF

  CHECK(tokens[0].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens[0]) == "hello");

  CHECK(tokens[1].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens[1]) == "world");

  CHECK(tokens[2].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens[2]) == "foo42");

  CHECK(tokens[3].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens[3]) == "_test");

  CHECK(tokens[4].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens[4]) == "variable_name");

  CHECK(tokens[5].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can tokenize keywords", "[lexer][phase1]") {
  LexerTestHelper helper;
  auto tokens =
      helper.tokenize("if else while for func return true false null");

  REQUIRE(tokens.size() == 10); // 9 keywords + EOF
  CHECK(tokens[0].kind == TokenKind::If);
  CHECK(tokens[1].kind == TokenKind::Else);
  CHECK(tokens[2].kind == TokenKind::While);
  CHECK(tokens[3].kind == TokenKind::For);
  CHECK(tokens[4].kind == TokenKind::Func);
  CHECK(tokens[5].kind == TokenKind::Return);
  CHECK(tokens[6].kind == TokenKind::True);
  CHECK(tokens[7].kind == TokenKind::False);
  CHECK(tokens[8].kind == TokenKind::Null);
  CHECK(tokens[9].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can tokenize basic integers", "[lexer][phase1]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("42 0 123 999");

  REQUIRE(tokens.size() == 5); // 4 integers + EOF

  CHECK(tokens[0].kind == TokenKind::IntLiteral);
  CHECK(tokens[0].value.intValue.value == 42);

  CHECK(tokens[1].kind == TokenKind::IntLiteral);
  CHECK(tokens[1].value.intValue.value == 0);

  CHECK(tokens[2].kind == TokenKind::IntLiteral);
  CHECK(tokens[2].value.intValue.value == 123);

  CHECK(tokens[3].kind == TokenKind::IntLiteral);
  CHECK(tokens[3].value.intValue.value == 999);

  CHECK(tokens[4].kind == TokenKind::EoF);
}

TEST_CASE("Lexer handles whitespace correctly", "[lexer][phase1]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("  \t\n  hello   \r\n  world  \t  ");

  REQUIRE(tokens.size() == 3); // 2 identifiers + EOF
  CHECK(tokens[0].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens[0]) == "hello");
  CHECK(tokens[1].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens[1]) == "world");
  CHECK(tokens[2].kind == TokenKind::EoF);
}

TEST_CASE("Lexer tracks source location correctly", "[lexer][phase1]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("hello\nworld");

  REQUIRE(tokens.size() == 3); // 2 identifiers + EOF

  // First token should be on line 1
  CHECK(tokens[0].location.start.row == 1);
  CHECK(tokens[0].location.start.column == 1);

  // Second token should be on line 2
  CHECK(tokens[1].location.start.row == 2);
  CHECK(tokens[1].location.start.column == 1);
}

TEST_CASE("Lexer can tokenize simple expressions", "[lexer][phase1]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("x + 42");

  REQUIRE(tokens.size() == 4); // ident + plus + integer + EOF
  CHECK(tokens[0].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens[0]) == "x");
  CHECK(tokens[1].kind == TokenKind::Plus);
  CHECK(tokens[2].kind == TokenKind::IntLiteral);
  CHECK(tokens[2].value.intValue.value == 42);
  CHECK(tokens[3].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can tokenize comparison expressions", "[lexer][phase1]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("foo == bar");

  REQUIRE(tokens.size() == 4); // ident + == + ident + EOF
  CHECK(tokens[0].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens[0]) == "foo");
  CHECK(tokens[1].kind == TokenKind::Equal);
  CHECK(tokens[2].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens[2]) == "bar");
  CHECK(tokens[3].kind == TokenKind::EoF);
}

TEST_CASE("Lexer handles empty input", "[lexer][phase1]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("");

  REQUIRE(tokens.size() == 1); // Just EOF
  CHECK(tokens[0].kind == TokenKind::EoF);
}

TEST_CASE("Lexer handles invalid characters with error recovery",
          "[lexer][phase1]") {
  LexerTestHelper helper;
  // Use a truly invalid character like $ which is not in our symbol list
  auto tokens = helper.tokenize("hello $ world");

  REQUIRE(tokens.size() >= 4); // At least hello + error + world + EOF
  CHECK(tokens[0].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens[0]) == "hello");
  CHECK(tokens[1].kind == TokenKind::Error);
  CHECK(tokens[2].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens[2]) == "world");

  // Check diagnostics
  CHECK(helper.hasErrors());
  CHECK(helper.getErrorCount() == 1);
  CHECK(helper.hasErrorContaining("Invalid character"));
}

// ============================================================================
// Phase 2: Integer Literals Tests
// ============================================================================

TEST_CASE("Lexer can parse decimal integers", "[lexer][phase2]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("42 0 123456789");

  REQUIRE(tokens.size() == 4); // 3 integers + EOF
  CHECK(tokens[0].kind == TokenKind::IntLiteral);
  CHECK(tokens[0].getIntValue() == 42);
  CHECK(tokens[0].getIntType() == IntegerKind::Auto);

  CHECK(tokens[1].kind == TokenKind::IntLiteral);
  CHECK(tokens[1].getIntValue() == 0);

  CHECK(tokens[2].kind == TokenKind::IntLiteral);
  CHECK(tokens[2].getIntValue() == 123456789);

  CHECK(tokens[3].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse hexadecimal integers", "[lexer][phase2]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("0x2A 0XFF 0x0");

  REQUIRE(tokens.size() == 4); // 3 hex integers + EOF
  CHECK(tokens[0].kind == TokenKind::IntLiteral);
  CHECK(tokens[0].getIntValue() == 0x2A);

  CHECK(tokens[1].kind == TokenKind::IntLiteral);
  CHECK(tokens[1].getIntValue() == 0xFF);

  CHECK(tokens[2].kind == TokenKind::IntLiteral);
  CHECK(tokens[2].getIntValue() == 0x0);

  CHECK(tokens[3].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse binary integers", "[lexer][phase2]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("0b1010 0B1111 0b0");

  REQUIRE(tokens.size() == 4); // 3 binary integers + EOF
  CHECK(tokens[0].kind == TokenKind::IntLiteral);
  CHECK(tokens[0].getIntValue() == 0b1010);

  CHECK(tokens[1].kind == TokenKind::IntLiteral);
  CHECK(tokens[1].getIntValue() == 0b1111);

  CHECK(tokens[2].kind == TokenKind::IntLiteral);
  CHECK(tokens[2].getIntValue() == 0b0);

  CHECK(tokens[3].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse octal integers", "[lexer][phase2]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("0o77 052 0o0");

  REQUIRE(tokens.size() == 4); // 3 octal integers + EOF
  CHECK(tokens[0].kind == TokenKind::IntLiteral);
  CHECK(tokens[0].getIntValue() == 077);

  CHECK(tokens[1].kind == TokenKind::IntLiteral);
  CHECK(tokens[1].getIntValue() == 052);

  CHECK(tokens[2].kind == TokenKind::IntLiteral);
  CHECK(tokens[2].getIntValue() == 0);

  CHECK(tokens[3].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse integers with underscore separators",
          "[lexer][phase2]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("1_000_000 0x_FF_AA 0b_1010_1010");

  REQUIRE(tokens.size() == 4); // 3 integers + EOF
  CHECK(tokens[0].kind == TokenKind::IntLiteral);
  CHECK(tokens[0].getIntValue() == 1000000);

  CHECK(tokens[1].kind == TokenKind::IntLiteral);
  CHECK(tokens[1].getIntValue() == 0xFFAA);

  CHECK(tokens[2].kind == TokenKind::IntLiteral);
  CHECK(tokens[2].getIntValue() == 0b10101010);

  CHECK(tokens[3].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse modern type suffixes", "[lexer][phase2]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("42i8 255u8 1000i16 65535u16 42i32 "
                                "4000000000u32 42i64 18446744073709551615u64");

  REQUIRE(tokens.size() == 9); // 8 integers + EOF

  // i8
  CHECK(tokens[0].kind == TokenKind::IntLiteral);
  CHECK(tokens[0].getIntValue() == 42);
  CHECK(tokens[0].getIntType() == IntegerKind::I8);

  // u8
  CHECK(tokens[1].kind == TokenKind::IntLiteral);
  CHECK(tokens[1].getIntValue() == 255);
  CHECK(tokens[1].getIntType() == IntegerKind::U8);

  // i16
  CHECK(tokens[2].kind == TokenKind::IntLiteral);
  CHECK(tokens[2].getIntValue() == 1000);
  CHECK(tokens[2].getIntType() == IntegerKind::I16);

  // u16
  CHECK(tokens[3].kind == TokenKind::IntLiteral);
  CHECK(tokens[3].getIntValue() == 65535);
  CHECK(tokens[3].getIntType() == IntegerKind::U16);

  // i32
  CHECK(tokens[4].kind == TokenKind::IntLiteral);
  CHECK(tokens[4].getIntValue() == 42);
  CHECK(tokens[4].getIntType() == IntegerKind::I32);

  // u32
  CHECK(tokens[5].kind == TokenKind::IntLiteral);
  CHECK(tokens[5].getIntValue() == 4000000000ULL);
  CHECK(tokens[5].getIntType() == IntegerKind::U32);

  // i64
  CHECK(tokens[6].kind == TokenKind::IntLiteral);
  CHECK(tokens[6].getIntValue() == 42);
  CHECK(tokens[6].getIntType() == IntegerKind::I64);

  // u64
  CHECK(tokens[7].kind == TokenKind::IntLiteral);
  CHECK(tokens[7].getIntValue() == 18446744073709551615ULL);
  CHECK(tokens[7].getIntType() == IntegerKind::U64);

  CHECK(tokens[8].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse legacy C-style suffixes", "[lexer][phase2]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("42u 42l 42ul 42ll 42ull");

  REQUIRE(tokens.size() == 6); // 5 integers + EOF

  // u -> U32
  CHECK(tokens[0].kind == TokenKind::IntLiteral);
  CHECK(tokens[0].getIntValue() == 42);
  CHECK(tokens[0].getIntType() == IntegerKind::U32);

  // l -> I64
  CHECK(tokens[1].kind == TokenKind::IntLiteral);
  CHECK(tokens[1].getIntValue() == 42);
  CHECK(tokens[1].getIntType() == IntegerKind::I64);

  // ul -> U64
  CHECK(tokens[2].kind == TokenKind::IntLiteral);
  CHECK(tokens[2].getIntValue() == 42);
  CHECK(tokens[2].getIntType() == IntegerKind::U64);

  // ll -> I64
  CHECK(tokens[3].kind == TokenKind::IntLiteral);
  CHECK(tokens[3].getIntValue() == 42);
  CHECK(tokens[3].getIntType() == IntegerKind::I64);

  // ull -> U64
  CHECK(tokens[4].kind == TokenKind::IntLiteral);
  CHECK(tokens[4].getIntValue() == 42);
  CHECK(tokens[4].getIntType() == IntegerKind::U64);

  CHECK(tokens[5].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse 128-bit integers", "[lexer][phase2]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("42i128 42u128");

  REQUIRE(tokens.size() == 3); // 2 integers + EOF

  CHECK(tokens[0].kind == TokenKind::IntLiteral);
  CHECK(tokens[0].getIntValue() == 42);
  CHECK(tokens[0].getIntType() == IntegerKind::I128);

  CHECK(tokens[1].kind == TokenKind::IntLiteral);
  CHECK(tokens[1].getIntValue() == 42);
  CHECK(tokens[1].getIntType() == IntegerKind::U128);

  CHECK(tokens[2].kind == TokenKind::EoF);
}

TEST_CASE("Lexer handles malformed integer literals", "[lexer][phase2]") {
  LexerTestHelper helper;

  // Test invalid hex (no digits after 0x)
  auto tokens1 = helper.tokenize("0x");
  CHECK(tokens1[0].kind == TokenKind::Error);
  CHECK(helper.hasErrors());
  CHECK(helper.hasErrorContaining("no digits"));

  helper.clearDiagnostics();

  // Test invalid binary (no digits after 0b)
  auto tokens2 = helper.tokenize("0b");
  CHECK(tokens2[0].kind == TokenKind::Error);
  CHECK(helper.hasErrors());
  CHECK(helper.hasErrorContaining("no digits"));

  helper.clearDiagnostics();

  // Test invalid type suffix
  auto tokens3 = helper.tokenize("42xyz");
  CHECK(tokens3[0].kind == TokenKind::IntLiteral); // Number parses
  CHECK(helper.hasErrors());
  CHECK(helper.hasErrorContaining("Invalid integer type suffix"));
  // Note: xyz would be treated as identifier after the number
}

TEST_CASE("Lexer can tokenize complex expressions with Phase 2 integers",
          "[lexer][phase2]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("0xFF + 42u32 - 0b1010i16");

  REQUIRE(tokens.size() == 6); // hex + plus + decimal + minus + binary + EOF

  CHECK(tokens[0].kind == TokenKind::IntLiteral);
  CHECK(tokens[0].getIntValue() == 0xFF);
  CHECK(tokens[0].getIntType() == IntegerKind::Auto);

  CHECK(tokens[1].kind == TokenKind::Plus);

  CHECK(tokens[2].kind == TokenKind::IntLiteral);
  CHECK(tokens[2].getIntValue() == 42);
  CHECK(tokens[2].getIntType() == IntegerKind::U32);

  CHECK(tokens[3].kind == TokenKind::Minus);

  CHECK(tokens[4].kind == TokenKind::IntLiteral);
  CHECK(tokens[4].getIntValue() == 0b1010);
  CHECK(tokens[4].getIntType() == IntegerKind::I16);

  CHECK(tokens[5].kind == TokenKind::EoF);
}

// ============================================================================
// Phase 3: Floating-Point Literals Tests
// ============================================================================

TEST_CASE("Lexer can parse basic decimal floats", "[lexer][phase3]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("3.14 0.5 5. 0.0");

  REQUIRE(tokens.size() == 5); // 4 floats + EOF
  CHECK(tokens[0].kind == TokenKind::FloatLiteral);
  CHECK(tokens[0].getFloatValue() == 3.14);
  CHECK(tokens[0].getFloatType() == FloatKind::Auto);

  CHECK(tokens[1].kind == TokenKind::FloatLiteral);
  CHECK(tokens[1].getFloatValue() == 0.5);

  CHECK(tokens[2].kind == TokenKind::FloatLiteral);
  CHECK(tokens[2].getFloatValue() == 5.0);

  CHECK(tokens[3].kind == TokenKind::FloatLiteral);
  CHECK(tokens[3].getFloatValue() == 0.0);

  CHECK(tokens[4].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse scientific notation", "[lexer][phase3]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("1e10 1.5e-3 2E+5 0.5e2");

  REQUIRE(tokens.size() == 5); // 4 floats + EOF
  CHECK(tokens[0].kind == TokenKind::FloatLiteral);
  CHECK(tokens[0].getFloatValue() == 1e10);

  CHECK(tokens[1].kind == TokenKind::FloatLiteral);
  CHECK(tokens[1].getFloatValue() == 1.5e-3);

  CHECK(tokens[2].kind == TokenKind::FloatLiteral);
  CHECK(tokens[2].getFloatValue() == 2e5);

  CHECK(tokens[3].kind == TokenKind::FloatLiteral);
  CHECK(tokens[3].getFloatValue() == 0.5e2);

  CHECK(tokens[4].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse hexadecimal floats", "[lexer][phase3]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("0x1.0 0x1.Ap+1 0xA.Fp-2");

  REQUIRE(tokens.size() == 4); // 3 hex floats + EOF
  CHECK(tokens[0].kind == TokenKind::FloatLiteral);
  CHECK(tokens[0].getFloatValue() == 1.0);

  CHECK(tokens[1].kind == TokenKind::FloatLiteral);
  // 0x1.A = 1 + 10/16 = 1.625, p+1 means *2^1 = 3.25
  CHECK(tokens[1].getFloatValue() == 3.25);

  CHECK(tokens[2].kind == TokenKind::FloatLiteral);
  // 0xA.F = 10 + 15/16 = 10.9375, p-2 means *2^-2 = 2.734375
  CHECK(tokens[2].getFloatValue() == Catch::Approx(2.734375));

  CHECK(tokens[3].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse floats with underscore separators",
          "[lexer][phase3]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("1_000.5_00 1_0e1_0");

  REQUIRE(tokens.size() == 3); // 2 floats + EOF
  CHECK(tokens[0].kind == TokenKind::FloatLiteral);
  CHECK(tokens[0].getFloatValue() == 1000.500);

  CHECK(tokens[1].kind == TokenKind::FloatLiteral);
  CHECK(tokens[1].getFloatValue() == 10e10);

  CHECK(tokens[2].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse float type suffixes", "[lexer][phase3]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("3.14f 2.0d 1.5F 0.5D");

  REQUIRE(tokens.size() == 5); // 4 floats + EOF

  // f suffix -> F32
  CHECK(tokens[0].kind == TokenKind::FloatLiteral);
  CHECK(tokens[0].getFloatValue() == Catch::Approx(3.14));
  CHECK(tokens[0].getFloatType() == FloatKind::F32);

  // d suffix -> F64
  CHECK(tokens[1].kind == TokenKind::FloatLiteral);
  CHECK(tokens[1].getFloatValue() == 2.0);
  CHECK(tokens[1].getFloatType() == FloatKind::F64);

  // F suffix -> F32
  CHECK(tokens[2].kind == TokenKind::FloatLiteral);
  CHECK(tokens[2].getFloatValue() == 1.5);
  CHECK(tokens[2].getFloatType() == FloatKind::F32);

  // D suffix -> F64
  CHECK(tokens[3].kind == TokenKind::FloatLiteral);
  CHECK(tokens[3].getFloatValue() == 0.5);
  CHECK(tokens[3].getFloatType() == FloatKind::F64);

  CHECK(tokens[4].kind == TokenKind::EoF);
}

TEST_CASE("Lexer handles malformed floating-point literals",
          "[lexer][phase3]") {
  LexerTestHelper helper;

  // Test invalid exponent (no digits after e)
  auto tokens1 = helper.tokenize("1.0e");
  CHECK(tokens1[0].kind == TokenKind::Error);

  // Test invalid hex exponent (no digits after p)
  auto tokens2 = helper.tokenize("0x1.0p");
  CHECK(tokens2[0].kind == TokenKind::Error);

  // Test invalid float suffix
  auto tokens3 = helper.tokenize("3.14xyz");
  CHECK(tokens3[0].kind == TokenKind::FloatLiteral); // Float parses
  // Note: xyz would be treated as identifier after the number
}

TEST_CASE("Lexer distinguishes integers from floats correctly",
          "[lexer][phase3]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("42 42.0 42f 42.5i32");

  REQUIRE(tokens.size() == 5); // 4 numbers + EOF

  // Integer
  CHECK(tokens[0].kind == TokenKind::IntLiteral);
  CHECK(tokens[0].getIntValue() == 42);

  // Float (has decimal point)
  CHECK(tokens[1].kind == TokenKind::FloatLiteral);
  CHECK(tokens[1].getFloatValue() == 42.0);

  // Float (has float suffix)
  CHECK(tokens[2].kind == TokenKind::FloatLiteral);
  CHECK(tokens[2].getFloatValue() == 42.0);
  CHECK(tokens[2].getFloatType() == FloatKind::F32);

  // Float (has decimal point, invalid integer suffix should be error)
  CHECK(tokens[3].kind == TokenKind::FloatLiteral);
  CHECK(tokens[3].getFloatValue() == 42.5);
  // The i32 suffix should be treated as invalid for float

  CHECK(tokens[4].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can tokenize complex expressions with Phase 3 floats",
          "[lexer][phase3]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("3.14f * 2.0 + 1e-5");

  REQUIRE(tokens.size() == 6); // float + mult + float + plus + float + EOF

  CHECK(tokens[0].kind == TokenKind::FloatLiteral);
  CHECK(tokens[0].getFloatValue() == Catch::Approx(3.14f));
  CHECK(tokens[0].getFloatType() == FloatKind::F32);

  CHECK(tokens[1].kind == TokenKind::Mult);

  CHECK(tokens[2].kind == TokenKind::FloatLiteral);
  CHECK(tokens[2].getFloatValue() == 2.0);
  CHECK(tokens[2].getFloatType() == FloatKind::Auto);

  CHECK(tokens[3].kind == TokenKind::Plus);

  CHECK(tokens[4].kind == TokenKind::FloatLiteral);
  CHECK(tokens[4].getFloatValue() == 1e-5);
  CHECK(tokens[4].getFloatType() == FloatKind::Auto);

  CHECK(tokens[5].kind == TokenKind::EoF);
}

// ============================================================================
// Phase 4: String and Character Literals Tests
// ============================================================================

TEST_CASE("Lexer can parse basic string literals", "[lexer][phase4]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("\"Hello, World!\" \"\" \"test\"");

  REQUIRE(tokens.size() == 4); // 3 strings + EOF
  CHECK(tokens[0].kind == TokenKind::StringLiteral);
  CHECK(tokens[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens[0]) == "Hello, World!");

  CHECK(tokens[1].kind == TokenKind::StringLiteral);
  CHECK(tokens[1].hasLiteralValue());
  CHECK(helper.getStringValue(tokens[1]) == "");

  CHECK(tokens[2].kind == TokenKind::StringLiteral);
  CHECK(tokens[2].hasLiteralValue());
  CHECK(helper.getStringValue(tokens[2]) == "test");

  CHECK(tokens[3].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse strings with standard escape sequences",
          "[lexer][phase4]") {
  LexerTestHelper helper;
  auto tokens =
      helper.tokenize(R"("Line 1\nLine 2\tTabbed\r\nWindows\\\"Quote")");

  REQUIRE(tokens.size() == 2); // 1 string + EOF
  CHECK(tokens[0].kind == TokenKind::StringLiteral);
  CHECK(tokens[0].hasLiteralValue());
  // Verify escape sequences are processed correctly
  CHECK(helper.getStringValue(tokens[0]) ==
        "Line 1\nLine 2\tTabbed\r\nWindows\\\"Quote");
  CHECK(tokens[1].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse strings with hex escape sequences",
          "[lexer][phase4]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize(R"("Byte: \xFF\x00\x41")");

  REQUIRE(tokens.size() == 2); // 1 string + EOF
  CHECK(tokens[0].kind == TokenKind::StringLiteral);
  CHECK(tokens[0].hasLiteralValue());
  // For now, hex escapes are treated as literal text (not implemented in escape
  // processor)
  CHECK(helper.getStringValue(tokens[0]) == "Byte: \\xFF\\x00\\x41");
  CHECK(tokens[1].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse strings with Unicode escapes", "[lexer][phase4]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize(R"("Unicode: \u{41}\u{1F680}")");

  REQUIRE(tokens.size() == 2); // 1 string + EOF
  CHECK(tokens[0].kind == TokenKind::StringLiteral);
  CHECK(tokens[0].hasLiteralValue());
  // Verify Unicode escapes are processed: \u{41} = 'A', \u{1F680} = rocket
  // emoji
  std::string expected = "Unicode: A🚀";
  CHECK(helper.getStringValue(tokens[0]) == expected);
  CHECK(tokens[1].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse basic character literals", "[lexer][phase4]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("'a' 'Z' '9'");

  REQUIRE(tokens.size() == 4); // 3 chars + EOF
  CHECK(tokens[0].kind == TokenKind::CharLiteral);
  CHECK(tokens[0].getCharValue() == static_cast<uint32_t>('a'));

  CHECK(tokens[1].kind == TokenKind::CharLiteral);
  CHECK(tokens[1].getCharValue() == static_cast<uint32_t>('Z'));

  CHECK(tokens[2].kind == TokenKind::CharLiteral);
  CHECK(tokens[2].getCharValue() == static_cast<uint32_t>('9'));

  CHECK(tokens[3].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse character literals with escape sequences",
          "[lexer][phase4]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize(R"('\n' '\t' '\\' '\'' '\"' '\0')");

  REQUIRE(tokens.size() == 7); // 6 chars + EOF
  CHECK(tokens[0].kind == TokenKind::CharLiteral);
  CHECK(tokens[0].getCharValue() == static_cast<uint32_t>('\n'));

  CHECK(tokens[1].kind == TokenKind::CharLiteral);
  CHECK(tokens[1].getCharValue() == static_cast<uint32_t>('\t'));

  CHECK(tokens[2].kind == TokenKind::CharLiteral);
  CHECK(tokens[2].getCharValue() == static_cast<uint32_t>('\\'));

  CHECK(tokens[3].kind == TokenKind::CharLiteral);
  CHECK(tokens[3].getCharValue() == static_cast<uint32_t>('\''));

  CHECK(tokens[4].kind == TokenKind::CharLiteral);
  CHECK(tokens[4].getCharValue() == static_cast<uint32_t>('\"'));

  CHECK(tokens[5].kind == TokenKind::CharLiteral);
  CHECK(tokens[5].getCharValue() == static_cast<uint32_t>('\0'));

  CHECK(tokens[6].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse character literals with hex escapes",
          "[lexer][phase4]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize(R"('\x41' '\xFF' '\x00')");

  REQUIRE(tokens.size() == 4); // 3 chars + EOF
  CHECK(tokens[0].kind == TokenKind::CharLiteral);
  CHECK(tokens[0].getCharValue() == 0x41); // 'A'

  CHECK(tokens[1].kind == TokenKind::CharLiteral);
  CHECK(tokens[1].getCharValue() == 0xFF);

  CHECK(tokens[2].kind == TokenKind::CharLiteral);
  CHECK(tokens[2].getCharValue() == 0x00);

  CHECK(tokens[3].kind == TokenKind::EoF);
}

TEST_CASE("Lexer can parse Unicode character literals", "[lexer][phase4]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize(R"('\u0041' '\u{41}')");

  REQUIRE(tokens.size() == 3); // 2 chars + EOF
  CHECK(tokens[0].kind == TokenKind::CharLiteral);
  CHECK(tokens[0].getCharValue() == 0x41); // 'A'

  CHECK(tokens[1].kind == TokenKind::CharLiteral);
  CHECK(tokens[1].getCharValue() == 0x41); // 'A'

  CHECK(tokens[2].kind == TokenKind::EoF);
}

TEST_CASE("Lexer handles UTF-8 characters correctly", "[lexer][phase4]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize("\"🚀\" '🚀'");

  REQUIRE(tokens.size() == 3); // string + char + EOF
  CHECK(tokens[0].kind == TokenKind::StringLiteral);
  CHECK(tokens[1].kind == TokenKind::CharLiteral);
  // The rocket emoji should be parsed as a valid Unicode codepoint
  CHECK(tokens[1].getCharValue() == 0x1F680);
  CHECK(tokens[2].kind == TokenKind::EoF);
}

TEST_CASE("Lexer handles malformed string literals", "[lexer][phase4]") {
  LexerTestHelper helper;

  // Test unterminated string
  auto tokens1 = helper.tokenize("\"unterminated");
  CHECK(tokens1[0].kind == TokenKind::Error);
  CHECK(helper.hasErrors());
  CHECK(helper.hasErrorContaining("Unterminated string literal"));

  helper.clearDiagnostics();

  // Test string with escaped newline (should work)
  auto tokens2 = helper.tokenize("\"line\\nbreak\"");
  CHECK(tokens2[0].kind == TokenKind::StringLiteral);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles malformed character literals", "[lexer][phase4]") {
  LexerTestHelper helper;

  // Test unterminated character
  auto tokens1 = helper.tokenize("'a");
  CHECK(tokens1[0].kind == TokenKind::Error);
  CHECK(helper.hasErrors());
  CHECK(helper.hasErrorContaining("Unterminated character literal"));

  helper.clearDiagnostics();

  // Test invalid escape
  auto tokens2 = helper.tokenize(R"('\z')");
  CHECK(tokens2[0].kind == TokenKind::Error);
  CHECK(helper.hasErrors());
  CHECK(helper.hasErrorContaining("Unknown escape sequence"));
}

TEST_CASE("Lexer can tokenize mixed expressions with strings and chars",
          "[lexer][phase4]") {
  LexerTestHelper helper;
  auto tokens = helper.tokenize(R"(print("Hello") + 'x')");

  REQUIRE(tokens.size() >= 6); // print + ( + string + ) + + + char + EOF

  // Find the string and character tokens
  bool foundString = false, foundChar = false;
  for (const auto &token : tokens) {
    if (token.kind == TokenKind::StringLiteral)
      foundString = true;
    if (token.kind == TokenKind::CharLiteral)
      foundChar = true;
  }

  CHECK(foundString);
  CHECK(foundChar);
}

TEST_CASE("Lexer handles multiline strings naturally", "[lexer][phase4]") {
  LexerTestHelper helper;

  // Test string with actual newlines in source
  std::string multilineSource = "\"Line 1\nLine 2\nLine 3\"";
  auto tokens = helper.tokenize(multilineSource);

  REQUIRE(tokens.size() == 2); // string + EOF
  CHECK(tokens[0].kind == TokenKind::StringLiteral);
  CHECK(tokens[1].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles raw string literals", "[lexer][phase4]") {
  LexerTestHelper helper;

  // Test basic raw string
  auto tokens1 = helper.tokenize("r\"C:\\Users\\path\\file.txt\"");
  REQUIRE(tokens1.size() == 2); // string + EOF
  CHECK(tokens1[0].kind == TokenKind::StringLiteral);
  CHECK(tokens1[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens1[0]) == "C:\\Users\\path\\file.txt");
  CHECK(tokens1[1].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());

  helper.clearDiagnostics();

  // Test raw string with quotes (no escape processing)
  auto tokens2 = helper.tokenize("r\"simple\"");
  REQUIRE(tokens2.size() == 2); // string + EOF
  CHECK(tokens2[0].kind == TokenKind::StringLiteral);
  CHECK(tokens2[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens2[0]) == "simple");
  CHECK(tokens2[1].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());

  helper.clearDiagnostics();

  // Test raw string with backslashes and quotes (all literal)
  auto tokens3 = helper.tokenize(R"#(r"Path: C:\Users\file.txt")#");
  REQUIRE(tokens3.size() == 2); // string + EOF
  CHECK(tokens3[0].kind == TokenKind::StringLiteral);
  CHECK(tokens3[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens3[0]) == "Path: C:\\Users\\file.txt");
  CHECK(tokens3[1].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles multiline raw strings", "[lexer][phase4]") {
  LexerTestHelper helper;

  // Test raw string spanning multiple lines
  std::string multilineRawSource =
      "r\"Line 1\nLine 2\n\\n literal backslash-n\"";
  auto tokens = helper.tokenize(multilineRawSource);

  REQUIRE(tokens.size() == 2); // string + EOF
  CHECK(tokens[0].kind == TokenKind::StringLiteral);
  CHECK(tokens[1].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles string literals with escape sequences and multiline",
          "[lexer][phase4]") {
  LexerTestHelper helper;

  // Test regular string with escapes that spans lines
  std::string mixedSource = "\"Hello\\nWorld\nActual newline\ntab:\\t\"";
  auto tokens = helper.tokenize(mixedSource);

  REQUIRE(tokens.size() == 2); // string + EOF
  CHECK(tokens[0].kind == TokenKind::StringLiteral);
  CHECK(tokens[1].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer distinguishes raw strings from regular identifiers",
          "[lexer][phase4]") {
  LexerTestHelper helper;

  // Test that 'r' followed by non-quote is still an identifier
  auto tokens1 = helper.tokenize("r");
  REQUIRE(tokens1.size() == 2); // ident + EOF
  CHECK(tokens1[0].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens1[0]) == "r");

  helper.clearDiagnostics();

  // Test that 'r' + other chars is an identifier
  auto tokens2 = helper.tokenize("raw_string");
  REQUIRE(tokens2.size() == 2); // ident + EOF
  CHECK(tokens2[0].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens2[0]) == "raw_string");

  helper.clearDiagnostics();

  // Test that only r" triggers raw string
  auto tokens3 = helper.tokenize("r\"raw\" normal");
  REQUIRE(tokens3.size() == 3); // raw string + ident + EOF
  CHECK(tokens3[0].kind == TokenKind::StringLiteral);
  CHECK(tokens3[1].kind == TokenKind::Ident);
  CHECK(helper.getTokenText(tokens3[1]) == "normal");

  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles mixed string types in expressions",
          "[lexer][phase4]") {
  LexerTestHelper helper;

  // Test mixing regular strings, raw strings, and other tokens
  auto tokens =
      helper.tokenize("func test() { \"regular\" + r\"raw\\nstring\" == 'c' }");

  bool foundRegularString = false, foundRawString = false, foundChar = false;
  for (const auto &token : tokens) {
    if (token.kind == TokenKind::StringLiteral) {
      // Both regular and raw strings use StringLiteral token kind
      if (!foundRegularString) {
        foundRegularString = true;
      } else {
        foundRawString = true;
      }
    }
    if (token.kind == TokenKind::CharLiteral) {
      foundChar = true;
    }
  }

  CHECK(foundRegularString);
  CHECK(foundRawString);
  CHECK(foundChar);
  CHECK_FALSE(helper.hasErrors());
}

// ============================================================================
// Additional Tests for Option 1 Implementation
// ============================================================================

TEST_CASE("Lexer Option 1: Stack vs Heap buffer strategy",
          "[lexer][phase4][option1]") {
  LexerTestHelper helper;

  // Test small string (should use stack buffer)
  auto tokens1 = helper.tokenize("\"Small string\"");
  REQUIRE(tokens1.size() == 2);
  CHECK(tokens1[0].kind == TokenKind::StringLiteral);
  CHECK(tokens1[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens1[0]) == "Small string");

  helper.clearDiagnostics();

  // Test large string (>512 bytes, should use heap buffer)
  std::string largeString = "\"" + std::string(600, 'A') + "\"";
  auto tokens2 = helper.tokenize(largeString);
  REQUIRE(tokens2.size() == 2);
  CHECK(tokens2[0].kind == TokenKind::StringLiteral);
  CHECK(tokens2[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens2[0]) == std::string(600, 'A'));

  helper.clearDiagnostics();

  // Test boundary case (exactly 512 bytes)
  std::string boundaryString = "\"" + std::string(512, 'B') + "\"";
  auto tokens3 = helper.tokenize(boundaryString);
  REQUIRE(tokens3.size() == 2);
  CHECK(tokens3[0].kind == TokenKind::StringLiteral);
  CHECK(tokens3[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens3[0]) == std::string(512, 'B'));

  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer Option 1: Large string with many escapes",
          "[lexer][phase4][option1]") {
  LexerTestHelper helper;

  // Test large string with many escape sequences (should use heap buffer)
  std::string largeStringWithEscapes;
  std::string expectedResult;

  // Build a string with 200 escape sequences
  for (int i = 0; i < 200; ++i) {
    largeStringWithEscapes += "\\n\\t\\r";
    expectedResult += "\n\t\r";
  }
  largeStringWithEscapes = "\"" + largeStringWithEscapes + "\"";

  auto tokens = helper.tokenize(largeStringWithEscapes);
  REQUIRE(tokens.size() == 2);
  CHECK(tokens[0].kind == TokenKind::StringLiteral);
  CHECK(tokens[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens[0]) == expectedResult);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer: Identifier interning verification",
          "[lexer][phase4][interning]") {
  LexerTestHelper helper;

  // Test that identical identifiers have the same interned string
  auto tokens = helper.tokenize("hello world hello foo world");

  REQUIRE(tokens.size() == 6); // 5 idents + EOF
  CHECK(tokens[0].kind == TokenKind::Ident);
  CHECK(tokens[1].kind == TokenKind::Ident);
  CHECK(tokens[2].kind == TokenKind::Ident);
  CHECK(tokens[3].kind == TokenKind::Ident);
  CHECK(tokens[4].kind == TokenKind::Ident);

  // Verify all identifiers have literal values
  CHECK(tokens[0].hasLiteralValue());
  CHECK(tokens[1].hasLiteralValue());
  CHECK(tokens[2].hasLiteralValue());
  CHECK(tokens[3].hasLiteralValue());
  CHECK(tokens[4].hasLiteralValue());

  // Verify string values
  CHECK(helper.getStringValue(tokens[0]) == "hello"); // first hello
  CHECK(helper.getStringValue(tokens[1]) == "world"); // first world
  CHECK(helper.getStringValue(tokens[2]) == "hello"); // second hello
  CHECK(helper.getStringValue(tokens[3]) == "foo");   // foo
  CHECK(helper.getStringValue(tokens[4]) == "world"); // second world

  // Verify that identical identifiers have the same InternedString object
  CHECK(tokens[0].value.stringValue ==
        tokens[2].value.stringValue); // both "hello"
  CHECK(tokens[1].value.stringValue ==
        tokens[4].value.stringValue); // both "world"

  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer: String token value verification", "[lexer][phase4][values]") {
  LexerTestHelper helper;

  auto tokens = helper.tokenize("\"processed\" r\"raw\" identifier");
  REQUIRE(tokens.size() == 4); // 2 strings + 1 ident + EOF

  // Test processed string
  CHECK(tokens[0].kind == TokenKind::StringLiteral);
  CHECK(tokens[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens[0]) == "processed");

  // Test raw string
  CHECK(tokens[1].kind == TokenKind::StringLiteral);
  CHECK(tokens[1].hasLiteralValue());
  CHECK(helper.getStringValue(tokens[1]) == "raw");

  // Test identifier
  CHECK(tokens[2].kind == TokenKind::Ident);
  CHECK(tokens[2].hasLiteralValue());
  CHECK(helper.getStringValue(tokens[2]) == "identifier");

  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer: Escape sequence edge cases", "[lexer][phase4][escapes]") {
  LexerTestHelper helper;

  // Test null character escape
  auto tokens1 = helper.tokenize("\"Hello\\0World\"");
  REQUIRE(tokens1.size() == 2);
  CHECK(tokens1[0].kind == TokenKind::StringLiteral);
  CHECK(tokens1[0].hasLiteralValue());
  std::string expected1 = "Hello";
  expected1 += '\0';
  expected1 += "World";
  CHECK(helper.getStringValue(tokens1[0]) == expected1);

  helper.clearDiagnostics();

  // Test escape at string boundaries
  auto tokens2 = helper.tokenize("\"\\nStart\" \"End\\t\"");
  REQUIRE(tokens2.size() == 3);
  CHECK(tokens2[0].kind == TokenKind::StringLiteral);
  CHECK(helper.getStringValue(tokens2[0]) == "\nStart");
  CHECK(tokens2[1].kind == TokenKind::StringLiteral);
  CHECK(helper.getStringValue(tokens2[1]) == "End\t");

  helper.clearDiagnostics();

  // Test mixed escape types
  auto tokens3 = helper.tokenize("\"\\n\\u{41}\\t\\u{42}\\r\"");
  REQUIRE(tokens3.size() == 2);
  CHECK(tokens3[0].kind == TokenKind::StringLiteral);
  CHECK(helper.getStringValue(tokens3[0]) == "\nA\tB\r");

  helper.clearDiagnostics();

  // Test unknown escape (should be treated literally)
  auto tokens4 = helper.tokenize("\"\\z\\q\"");
  REQUIRE(tokens4.size() == 2);
  CHECK(tokens4[0].kind == TokenKind::StringLiteral);
  CHECK(helper.getStringValue(tokens4[0]) == "\\z\\q");

  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer: Invalid Unicode escape handling",
          "[lexer][phase4][unicode]") {
  LexerTestHelper helper;

  // Test malformed Unicode escape (missing closing brace)
  auto tokens1 = helper.tokenize("\"\\u{41\"");
  REQUIRE(tokens1.size() == 2);
  CHECK(tokens1[0].kind == TokenKind::StringLiteral);
  // Should treat as literal since malformed
  CHECK(helper.getStringValue(tokens1[0]) == "\\u{41");

  helper.clearDiagnostics();

  // Test invalid Unicode escape (invalid hex digits)
  auto tokens2 = helper.tokenize("\"\\u{GGG}\"");
  REQUIRE(tokens2.size() == 2);
  CHECK(tokens2[0].kind == TokenKind::StringLiteral);
  // Should treat as literal since invalid
  CHECK(helper.getStringValue(tokens2[0]) == "\\u{GGG}");

  helper.clearDiagnostics();

  // Test Unicode escape with too large codepoint
  auto tokens3 = helper.tokenize("\"\\u{FFFFFF}\"");
  REQUIRE(tokens3.size() == 2);
  CHECK(tokens3[0].kind == TokenKind::StringLiteral);
  // Should treat as literal since > 0x10FFFF
  CHECK(helper.getStringValue(tokens3[0]) == "\\u{FFFFFF}");

  helper.clearDiagnostics();

  // Test valid Unicode escapes work correctly
  auto tokens4 = helper.tokenize("\"\\u{0}\\u{41}\\u{1F680}\"");
  REQUIRE(tokens4.size() == 2);
  CHECK(tokens4[0].kind == TokenKind::StringLiteral);
  std::string expected4;
  expected4 += '\0'; // U+0000
  expected4 += 'A';  // U+0041
  expected4 += "🚀"; // U+1F680
  CHECK(helper.getStringValue(tokens4[0]) == expected4);

  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer: String processing no escapes optimization",
          "[lexer][phase4][optimization]") {
  LexerTestHelper helper;

  // Test that strings without escapes are processed efficiently
  auto tokens1 = helper.tokenize("\"Simple string with no escapes\"");
  REQUIRE(tokens1.size() == 2);
  CHECK(tokens1[0].kind == TokenKind::StringLiteral);
  CHECK(tokens1[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens1[0]) == "Simple string with no escapes");

  helper.clearDiagnostics();

  // Test large string without escapes
  std::string largeNoEscape = "\"" + std::string(1000, 'X') + "\"";
  auto tokens2 = helper.tokenize(largeNoEscape);
  REQUIRE(tokens2.size() == 2);
  CHECK(tokens2[0].kind == TokenKind::StringLiteral);
  CHECK(tokens2[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens2[0]) == std::string(1000, 'X'));

  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer: Raw string edge cases", "[lexer][phase4][raw]") {
  LexerTestHelper helper;

  // Test raw string with escaped quotes (should be literal)
  auto tokens1 = helper.tokenize("r\"simple raw string\"");
  REQUIRE(tokens1.size() == 2);
  CHECK(tokens1[0].kind == TokenKind::StringLiteral);
  CHECK(tokens1[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens1[0]) == "simple raw string");

  helper.clearDiagnostics();

  // Test raw string with backslashes (should be literal)
  auto tokens2 = helper.tokenize("r\"C:\\Program Files\\Test\\file.txt\"");
  REQUIRE(tokens2.size() == 2);
  CHECK(tokens2[0].kind == TokenKind::StringLiteral);
  CHECK(tokens2[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens2[0]) ==
        "C:\\Program Files\\Test\\file.txt");

  helper.clearDiagnostics();

  // Test empty raw string
  auto tokens3 = helper.tokenize("r\"\"");
  REQUIRE(tokens3.size() == 2);
  CHECK(tokens3[0].kind == TokenKind::StringLiteral);
  CHECK(tokens3[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens3[0]) == "");

  helper.clearDiagnostics();

  // Test raw string with unicode sequences (should be literal)
  auto tokens4 = helper.tokenize("r\"\\u{41}\\n\\t\"");
  REQUIRE(tokens4.size() == 2);
  CHECK(tokens4[0].kind == TokenKind::StringLiteral);
  CHECK(tokens4[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens4[0]) == "\\u{41}\\n\\t");

  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer: Escaped braces in string literals", "[lexer][phase4]") {
  LexerTestHelper helper;

  // Test string with escaped braces (to avoid string interpolation)
  auto tokens = helper.tokenize("\"[]\\{\\}()<>\"");

  REQUIRE(tokens.size() == 2);
  CHECK(tokens[0].kind == TokenKind::StringLiteral);
  CHECK(tokens[0].hasLiteralValue());
  CHECK(helper.getStringValue(tokens[0]) == "[]{}()<>");
  CHECK_FALSE(helper.hasErrors());
}

// Phase 5: Comments Tests
TEST_CASE("Lexer handles line comments", "[lexer][phase5]") {
  LexerTestHelper helper;

  // Test basic line comment
  auto tokens1 = helper.tokenize("x = 42; // This is a comment");

  // Should only tokenize up to the comment
  REQUIRE(tokens1.size() == 5); // x + = + 42 + ; + EOF (comment is skipped)
  CHECK(tokens1[0].kind == TokenKind::Ident);
  CHECK(tokens1[1].kind == TokenKind::Assign);
  CHECK(tokens1[2].kind == TokenKind::IntLiteral);
  CHECK(tokens1[3].kind == TokenKind::Semicolon);
  CHECK(tokens1[4].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());

  helper.clearDiagnostics();

  // Test comment with special characters
  auto tokens2 =
      helper.tokenize("func test() { // Comment with symbols!@#$%^&*()");

  // Should tokenize the code but skip the comment
  REQUIRE(tokens2.size() == 6);              // func + test + ( + ) + { + EOF
  CHECK(tokens2[0].kind == TokenKind::Func); // func (keyword)
  CHECK(tokens2[1].kind == TokenKind::Test); // test (keyword)
  CHECK(tokens2[2].kind == TokenKind::LParen);
  CHECK(tokens2[3].kind == TokenKind::RParen);
  CHECK(tokens2[4].kind == TokenKind::LBrace);
  CHECK(tokens2[5].kind == TokenKind::EoF);
  // Comment should be completely skipped
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles block comments", "[lexer][phase5]") {
  LexerTestHelper helper;

  // Test basic block comment
  auto tokens1 = helper.tokenize("x = /* comment */ 42;");

  REQUIRE(tokens1.size() == 5); // x + = + 42 + ; + EOF
  CHECK(tokens1[0].kind == TokenKind::Ident);
  CHECK(tokens1[1].kind == TokenKind::Assign);
  CHECK(tokens1[2].kind == TokenKind::IntLiteral);
  CHECK(tokens1[3].kind == TokenKind::Semicolon);
  CHECK(tokens1[4].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());

  helper.clearDiagnostics();

  // Test multiline block comment
  std::string multilineComment = "x = /*\\nMultiline\\ncomment\\n*/ 42;";
  auto tokens2 = helper.tokenize(multilineComment);

  REQUIRE(tokens2.size() == 5); // x + = + 42 + ; + EOF
  CHECK(tokens2[0].kind == TokenKind::Ident);
  CHECK(tokens2[1].kind == TokenKind::Assign);
  CHECK(tokens2[2].kind == TokenKind::IntLiteral);
  CHECK(tokens2[3].kind == TokenKind::Semicolon);
  CHECK(tokens2[4].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles nested block comments", "[lexer][phase5]") {
  LexerTestHelper helper;

  // Test nested block comments
  auto tokens = helper.tokenize("x = /* outer /* inner */ outer */ 42;");

  REQUIRE(tokens.size() == 5); // x + = + 42 + ; + EOF
  CHECK(tokens[0].kind == TokenKind::Ident);
  CHECK(tokens[1].kind == TokenKind::Assign);
  CHECK(tokens[2].kind == TokenKind::IntLiteral);
  CHECK(tokens[3].kind == TokenKind::Semicolon);
  CHECK(tokens[4].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());

  helper.clearDiagnostics();

  // Test deeply nested comments
  auto tokens2 = helper.tokenize(
      "/* level 1 /* level 2 /* level 3 */ level 2 */ level 1 */ x;");

  REQUIRE(tokens2.size() == 3); // x + ; + EOF
  CHECK(tokens2[0].kind == TokenKind::Ident);
  CHECK(tokens2[1].kind == TokenKind::Semicolon);
  CHECK(tokens2[2].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles mixed comments", "[lexer][phase5]") {
  LexerTestHelper helper;

  // Test line comment followed by block comment
  std::string mixed = "x = 42; // line comment\n/* block comment */ y = 24;";
  auto tokens = helper.tokenize(mixed);

  // Should have: x = 42 ; y = 24 ; EOF
  bool foundX = false, foundY = false;
  int literalCount = 0;

  for (const auto &token : tokens) {
    if (token.kind == TokenKind::Ident) {
      std::string text{helper.getTokenText(token)};
      if (text == "x")
        foundX = true;
      if (text == "y")
        foundY = true;
    }
    if (token.kind == TokenKind::IntLiteral) {
      literalCount++;
    }
  }

  CHECK(foundX);
  CHECK(foundY);
  CHECK(literalCount == 2); // 42 and 24
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles comments in strings correctly", "[lexer][phase5]") {
  LexerTestHelper helper;

  // Test that comment markers inside strings are not treated as comments
  auto tokens1 = helper.tokenize("s = \"This /* is not */ a comment\";");

  REQUIRE(tokens1.size() == 5); // s + = + string + ; + EOF
  CHECK(tokens1[0].kind == TokenKind::Ident);
  CHECK(tokens1[1].kind == TokenKind::Assign);
  CHECK(tokens1[2].kind == TokenKind::StringLiteral);
  CHECK(tokens1[3].kind == TokenKind::Semicolon);
  CHECK(tokens1[4].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());

  helper.clearDiagnostics();

  // Test line comment markers in strings
  auto tokens2 = helper.tokenize("s = \"This // is not a comment\";");

  REQUIRE(tokens2.size() == 5); // s + = + string + ; + EOF
  CHECK(tokens2[0].kind == TokenKind::Ident);
  CHECK(tokens2[1].kind == TokenKind::Assign);
  CHECK(tokens2[2].kind == TokenKind::StringLiteral);
  CHECK(tokens2[3].kind == TokenKind::Semicolon);
  CHECK(tokens2[4].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles unterminated block comments", "[lexer][phase5]") {
  LexerTestHelper helper;

  // Test unterminated block comment
  auto tokens = helper.tokenize("x = /* unterminated comment");

  // Should still tokenize what it can and report error
  CHECK(helper.hasErrors());
  CHECK(helper.hasErrorContaining("Unterminated block comment"));
}

TEST_CASE("Lexer handles comments with operators", "[lexer][phase5]") {
  LexerTestHelper helper;

  // Test that division operators are distinguished from comment starts
  auto tokens1 = helper.tokenize("x = a / b; // Real comment");

  bool foundDiv = false;
  for (const auto &token : tokens1) {
    if (token.kind == TokenKind::Div) {
      foundDiv = true;
    }
  }

  CHECK(foundDiv);
  CHECK_FALSE(helper.hasErrors());

  helper.clearDiagnostics();

  // Test compound assignment with division
  auto tokens2 = helper.tokenize("x /= 2; /* Block comment */");

  bool foundDivEqual = false;
  for (const auto &token : tokens2) {
    if (token.kind == TokenKind::DivEqual) {
      foundDivEqual = true;
    }
  }

  CHECK(foundDivEqual);
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles empty comments", "[lexer][phase5]") {
  LexerTestHelper helper;

  // Test empty line comment
  auto tokens1 = helper.tokenize("x = 42;//");

  REQUIRE(tokens1.size() == 5); // x + = + 42 + ; + EOF
  CHECK(tokens1[0].kind == TokenKind::Ident);
  CHECK(tokens1[1].kind == TokenKind::Assign);
  CHECK(tokens1[2].kind == TokenKind::IntLiteral);
  CHECK(tokens1[3].kind == TokenKind::Semicolon);
  CHECK(tokens1[4].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());

  helper.clearDiagnostics();

  // Test empty block comment
  auto tokens2 = helper.tokenize("x = /**/ 42;");

  REQUIRE(tokens2.size() == 5); // x + = + 42 + ; + EOF
  CHECK(tokens2[0].kind == TokenKind::Ident);
  CHECK(tokens2[1].kind == TokenKind::Assign);
  CHECK(tokens2[2].kind == TokenKind::IntLiteral);
  CHECK(tokens2[3].kind == TokenKind::Semicolon);
  CHECK(tokens2[4].kind == TokenKind::EoF);
  CHECK_FALSE(helper.hasErrors());
}

// Phase 1.5: Advanced Operators and Symbols Tests
TEST_CASE("Lexer handles compound assignment operators", "[lexer][phase1.5]") {
  LexerTestHelper helper;

  // Test basic compound assignments
  auto tokens = helper.tokenize("a += b -= c *= d /= e %= f");

  std::vector<TokenKind> expected = {TokenKind::Ident,      // a
                                     TokenKind::PlusEqual,  // +=
                                     TokenKind::Ident,      // b
                                     TokenKind::MinusEqual, // -=
                                     TokenKind::Ident,      // c
                                     TokenKind::MultEqual,  // *=
                                     TokenKind::Ident,      // d
                                     TokenKind::DivEqual,   // /=
                                     TokenKind::Ident,      // e
                                     TokenKind::ModEqual,   // %=
                                     TokenKind::Ident,      // f
                                     TokenKind::EoF};

  REQUIRE(tokens.size() == expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    CHECK(tokens[i].kind == expected[i]);
  }
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles bitwise compound assignment operators",
          "[lexer][phase1.5]") {
  LexerTestHelper helper;

  auto tokens = helper.tokenize("a &= b |= c ^= d <<= e >>= f");

  std::vector<TokenKind> expected = {TokenKind::Ident,     // a
                                     TokenKind::BAndEqual, // &=
                                     TokenKind::Ident,     // b
                                     TokenKind::BOrEqual,  // |=
                                     TokenKind::Ident,     // c
                                     TokenKind::BXorEqual, // ^=
                                     TokenKind::Ident,     // d
                                     TokenKind::ShlEqual,  // <<=
                                     TokenKind::Ident,     // e
                                     TokenKind::ShrEqual,  // >>=
                                     TokenKind::Ident,     // f
                                     TokenKind::EoF};

  REQUIRE(tokens.size() == expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    CHECK(tokens[i].kind == expected[i]);
  }
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles increment and decrement operators",
          "[lexer][phase1.5]") {
  LexerTestHelper helper;

  auto tokens = helper.tokenize("++a b-- --c d++");

  std::vector<TokenKind> expected = {TokenKind::PlusPlus,   // ++
                                     TokenKind::Ident,      // a
                                     TokenKind::Ident,      // b
                                     TokenKind::MinusMinus, // --
                                     TokenKind::MinusMinus, // --
                                     TokenKind::Ident,      // c
                                     TokenKind::Ident,      // d
                                     TokenKind::PlusPlus,   // ++
                                     TokenKind::EoF};

  REQUIRE(tokens.size() == expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    CHECK(tokens[i].kind == expected[i]);
  }
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles arrow operators", "[lexer][phase1.5]") {
  LexerTestHelper helper;

  auto tokens = helper.tokenize("a -> b => c");

  std::vector<TokenKind> expected = {TokenKind::Ident,     // a
                                     TokenKind::ThinArrow, // ->
                                     TokenKind::Ident,     // b
                                     TokenKind::FatArrow,  // =>
                                     TokenKind::Ident,     // c
                                     TokenKind::EoF};

  REQUIRE(tokens.size() == expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    CHECK(tokens[i].kind == expected[i]);
  }
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles bitwise operators", "[lexer][phase1.5]") {
  LexerTestHelper helper;

  auto tokens = helper.tokenize("a & b | c ^ d ~ e << f >> g");

  std::vector<TokenKind> expected = {TokenKind::Ident, // a
                                     TokenKind::BAnd,  // &
                                     TokenKind::Ident, // b
                                     TokenKind::BOr,   // |
                                     TokenKind::Ident, // c
                                     TokenKind::BXor,  // ^
                                     TokenKind::Ident, // d
                                     TokenKind::BNot,  // ~
                                     TokenKind::Ident, // e
                                     TokenKind::Shl,   // <<
                                     TokenKind::Ident, // f
                                     TokenKind::Shr,   // >>
                                     TokenKind::Ident, // g
                                     TokenKind::EoF};

  REQUIRE(tokens.size() == expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    CHECK(tokens[i].kind == expected[i]);
  }
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles logical operators", "[lexer][phase1.5]") {
  LexerTestHelper helper;

  auto tokens = helper.tokenize("a && b || c");

  std::vector<TokenKind> expected = {TokenKind::Ident, // a
                                     TokenKind::LAnd,  // &&
                                     TokenKind::Ident, // b
                                     TokenKind::LOr,   // ||
                                     TokenKind::Ident, // c
                                     TokenKind::EoF};

  REQUIRE(tokens.size() == expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    CHECK(tokens[i].kind == expected[i]);
  }
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles dot operators", "[lexer][phase1.5]") {
  LexerTestHelper helper;

  auto tokens = helper.tokenize("a.b..c...d");

  std::vector<TokenKind> expected = {TokenKind::Ident,   // a
                                     TokenKind::Dot,     // .
                                     TokenKind::Ident,   // b
                                     TokenKind::DotDot,  // ..
                                     TokenKind::Ident,   // c
                                     TokenKind::Elipsis, // ...
                                     TokenKind::Ident,   // d
                                     TokenKind::EoF};

  REQUIRE(tokens.size() == expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    CHECK(tokens[i].kind == expected[i]);
  }
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles special punctuation", "[lexer][phase1.5]") {
  LexerTestHelper helper;

  auto tokens = helper.tokenize("@ # ? : ` !:");

  std::vector<TokenKind> expected = {TokenKind::At,        // @
                                     TokenKind::Hash,      // #
                                     TokenKind::Question,  // ?
                                     TokenKind::Colon,     // :
                                     TokenKind::Quote,     // `
                                     TokenKind::BangColon, // !:
                                     TokenKind::EoF};

  REQUIRE(tokens.size() == expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    CHECK(tokens[i].kind == expected[i]);
  }
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles advanced punctuation and macros",
          "[lexer][phase1.5]") {
  LexerTestHelper helper;

  auto tokens = helper.tokenize("## #. &.");

  std::vector<TokenKind> expected = {TokenKind::Define,         // ##
                                     TokenKind::AstMacroAccess, // #.
                                     TokenKind::BAndDot,        // &.
                                     TokenKind::EoF};

  REQUIRE(tokens.size() == expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    CHECK(tokens[i].kind == expected[i]);
  }
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles operator overload tokens", "[lexer][phase1.5]") {
  LexerTestHelper helper;

  auto tokens = helper.tokenize("() [] []=");

  std::vector<TokenKind> expected = {TokenKind::LParen,   // (
                                     TokenKind::RParen,   // )
                                     TokenKind::LBracket, // [
                                     TokenKind::RBracket, // ]
                                     TokenKind::LBracket, // [
                                     TokenKind::RBracket, // ]
                                     TokenKind::Assign,   // =
                                     TokenKind::EoF};

  REQUIRE(tokens.size() == expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    CHECK(tokens[i].kind == expected[i]);
  }
  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer distinguishes between similar operators",
          "[lexer][phase1.5]") {
  LexerTestHelper helper;

  // Test that we distinguish + vs ++ vs +=
  auto tokens1 = helper.tokenize("+ ++ +=");
  std::vector<TokenKind> expected1 = {TokenKind::Plus, TokenKind::PlusPlus,
                                      TokenKind::PlusEqual, TokenKind::EoF};
  REQUIRE(tokens1.size() == expected1.size());
  for (size_t i = 0; i < expected1.size(); ++i) {
    CHECK(tokens1[i].kind == expected1[i]);
  }

  helper.clearDiagnostics();

  // Test that we distinguish < vs << vs <= vs <<=
  auto tokens2 = helper.tokenize("< << <= <<=");
  std::vector<TokenKind> expected2 = {TokenKind::Less, TokenKind::Shl,
                                      TokenKind::LessEqual, TokenKind::ShlEqual,
                                      TokenKind::EoF};
  REQUIRE(tokens2.size() == expected2.size());
  for (size_t i = 0; i < expected2.size(); ++i) {
    CHECK(tokens2[i].kind == expected2[i]);
  }

  helper.clearDiagnostics();

  // Test that we distinguish & vs && vs &= vs &.
  auto tokens3 = helper.tokenize("& && &= &.");
  std::vector<TokenKind> expected3 = {TokenKind::BAnd, TokenKind::LAnd,
                                      TokenKind::BAndEqual, TokenKind::BAndDot,
                                      TokenKind::EoF};
  REQUIRE(tokens3.size() == expected3.size());
  for (size_t i = 0; i < expected3.size(); ++i) {
    CHECK(tokens3[i].kind == expected3[i]);
  }

  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Lexer handles complex operator expressions", "[lexer][phase1.5]") {
  LexerTestHelper helper;

  // Test a complex expression with many operators
  auto tokens = helper.tokenize("a += b++ << c &= d->e => f() || g[h]");

  // Should tokenize correctly without errors
  CHECK_FALSE(helper.hasErrors());

  // Should have the correct number of tokens (not going to verify every one,
  // just basics)
  bool foundPlusEqual = false, foundPlusPlus = false, foundShl = false;
  bool foundBAndEqual = false, foundThinArrow = false, foundFatArrow = false;
  bool foundLParen = false, foundLOr = false, foundLBracket = false;

  for (const auto &token : tokens) {
    switch (token.kind) {
    case TokenKind::PlusEqual:
      foundPlusEqual = true;
      break;
    case TokenKind::PlusPlus:
      foundPlusPlus = true;
      break;
    case TokenKind::Shl:
      foundShl = true;
      break;
    case TokenKind::BAndEqual:
      foundBAndEqual = true;
      break;
    case TokenKind::ThinArrow:
      foundThinArrow = true;
      break;
    case TokenKind::FatArrow:
      foundFatArrow = true;
      break;
    case TokenKind::LOr:
      foundLOr = true;
      break;
    case TokenKind::LParen:
      foundLParen = true;
      break;
    case TokenKind::LBracket:
      foundLBracket = true;
      break;
    default:
      break;
    }
  }

  CHECK(foundPlusEqual);
  CHECK(foundPlusPlus);
  CHECK(foundShl);
  CHECK(foundBAndEqual);
  CHECK(foundThinArrow);
  CHECK(foundFatArrow);
  CHECK(foundLParen);
  CHECK(foundLOr);
  CHECK(foundLBracket);
}

} // namespace cxy
