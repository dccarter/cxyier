#include "../parser_test_utils.hpp"

#include "catch2.hpp"

using namespace cxy;
using namespace cxy::test;

TEST_CASE("Parser: Token buffer initialization", "[parser][buffer]") {
  auto fixture = createParserFixture("42 3.14 'a'");

  // After initialization, buffer should be:
  // tokens_[0] = Error (no previous)
  // tokens_[1] = IntLiteral(42) - current
  // tokens_[2] = FloatLiteral(3.14) - lookahead1
  // tokens_[3] = CharLiteral('a') - lookahead2

  REQUIRE(fixture->current().kind == TokenKind::IntLiteral);
  REQUIRE(fixture->lookahead(1).kind == TokenKind::FloatLiteral);
  REQUIRE(fixture->lookahead(2).kind == TokenKind::CharLiteral);

  // Previous should be error token initially
  REQUIRE(fixture->parser().previous().kind == TokenKind::Error);
}

TEST_CASE("Parser: Token buffer advancement", "[parser][buffer]") {
  auto fixture = createParserFixture("42 3.14 'a' \"hello\"");

  // Initial state
  checkTokenBuffer(fixture->parser(), TokenKind::IntLiteral,
                   TokenKind::FloatLiteral, TokenKind::CharLiteral);

  // After first advance
  advanceAndCheck(fixture->parser(), TokenKind::FloatLiteral,
                  TokenKind::CharLiteral, TokenKind::StringLiteral);

  // Previous should now be IntLiteral
  REQUIRE(fixture->parser().previous().kind == TokenKind::IntLiteral);

  // After second advance
  advanceAndCheck(fixture->parser(), TokenKind::CharLiteral,
                  TokenKind::StringLiteral, TokenKind::EoF);

  // Previous should now be FloatLiteral
  REQUIRE(fixture->parser().previous().kind == TokenKind::FloatLiteral);

  // After third advance
  advanceAndCheck(fixture->parser(), TokenKind::StringLiteral, TokenKind::EoF,
                  TokenKind::EoF);

  // After fourth advance (EOF)
  advanceAndCheck(fixture->parser(), TokenKind::EoF, TokenKind::EoF,
                  TokenKind::EoF);

  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Lookahead access validation", "[parser][buffer]") {
  auto fixture = createParserFixture("a b c");

  // Valid lookahead offsets (1 and 2)
  REQUIRE(fixture->lookahead(1).kind == TokenKind::Ident);
  REQUIRE(fixture->lookahead(2).kind == TokenKind::Ident);

  // Invalid lookahead offsets should return empty token
  REQUIRE(fixture->lookahead(0).kind == TokenKind::Error);
  REQUIRE(fixture->lookahead(3).kind == TokenKind::Error);
  REQUIRE(fixture->lookahead(-1).kind == TokenKind::Error);
}

TEST_CASE("Parser: Empty input handling", "[parser][buffer]") {
  auto fixture = createParserFixture("");

  // Should immediately be at EOF
  REQUIRE(fixture->current().kind == TokenKind::EoF);
  REQUIRE(fixture->lookahead(1).kind == TokenKind::EoF);
  REQUIRE(fixture->lookahead(2).kind == TokenKind::EoF);
  REQUIRE(fixture->isAtEnd());

  // Advancing at EOF should stay at EOF
  fixture->advance();
  REQUIRE(fixture->current().kind == TokenKind::EoF);
  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Single token input", "[parser][buffer]") {
  auto fixture = createParserFixture("42");

  // Initial state
  REQUIRE(fixture->current().kind == TokenKind::IntLiteral);
  REQUIRE(fixture->lookahead(1).kind == TokenKind::EoF);
  REQUIRE(fixture->lookahead(2).kind == TokenKind::EoF);
  REQUIRE_FALSE(fixture->isAtEnd());

  // After advance
  fixture->advance();
  REQUIRE(fixture->current().kind == TokenKind::EoF);
  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Token buffer with whitespace", "[parser][buffer]") {
  auto fixture = createParserFixture("  42   3.14  \n  'a'  ");

  // Lexer should skip whitespace, so buffer contains only meaningful tokens
  checkTokenBuffer(fixture->parser(), TokenKind::IntLiteral,
                   TokenKind::FloatLiteral, TokenKind::CharLiteral);

  advanceAndCheck(fixture->parser(), TokenKind::FloatLiteral,
                  TokenKind::CharLiteral, TokenKind::EoF);

  advanceAndCheck(fixture->parser(), TokenKind::CharLiteral, TokenKind::EoF,
                  TokenKind::EoF);
}

TEST_CASE("Parser: check() method", "[parser][buffer]") {
  auto fixture = createParserFixture("42 3.14");

  REQUIRE(fixture->parser().check(TokenKind::IntLiteral));
  REQUIRE_FALSE(fixture->parser().check(TokenKind::FloatLiteral));
  REQUIRE_FALSE(fixture->parser().check(TokenKind::EoF));

  fixture->advance();
  REQUIRE(fixture->parser().check(TokenKind::FloatLiteral));
  REQUIRE_FALSE(fixture->parser().check(TokenKind::IntLiteral));
}

TEST_CASE("Parser: checkAny() method", "[parser][buffer]") {
  auto fixture = createParserFixture("42");

  std::vector<TokenKind> literals = {
      TokenKind::IntLiteral, TokenKind::FloatLiteral, TokenKind::StringLiteral};

  std::vector<TokenKind> keywords = {TokenKind::True, TokenKind::False,
                                     TokenKind::Null};

  REQUIRE(fixture->parser().checkAny(literals));
  REQUIRE_FALSE(fixture->parser().checkAny(keywords));

  // Empty vector should return false
  std::vector<TokenKind> empty;
  REQUIRE_FALSE(fixture->parser().checkAny(empty));
}

TEST_CASE("Parser: match() method", "[parser][buffer]") {
  auto fixture = createParserFixture("42 3.14");

  // Match successful - should advance and return true
  REQUIRE(fixture->parser().match(TokenKind::IntLiteral));
  REQUIRE(fixture->current().kind == TokenKind::FloatLiteral);

  // Match unsuccessful - should not advance and return false
  REQUIRE_FALSE(fixture->parser().match(TokenKind::IntLiteral));
  REQUIRE(fixture->current().kind == TokenKind::FloatLiteral);

  // Match successful again
  REQUIRE(fixture->parser().match(TokenKind::FloatLiteral));
  REQUIRE(fixture->current().kind == TokenKind::EoF);
}

TEST_CASE("Parser: expect() method success", "[parser][buffer]") {
  auto fixture = createParserFixture("42 3.14");

  // Successful expectation - should advance and return true
  REQUIRE(fixture->parser().expect(TokenKind::IntLiteral));
  REQUIRE(fixture->current().kind == TokenKind::FloatLiteral);

  // Another successful expectation
  REQUIRE(fixture->parser().expect(TokenKind::FloatLiteral,
                                   "Custom error message"));
  REQUIRE(fixture->current().kind == TokenKind::EoF);
}

TEST_CASE("Parser: expect() method failure", "[parser][buffer]") {
  auto fixture = createParserFixture("42");

  // Failed expectation - should not advance and return false
  REQUIRE_FALSE(fixture->parser().expect(TokenKind::StringLiteral));
  REQUIRE(fixture->current().kind == TokenKind::IntLiteral);

  // Should still be able to consume the correct token
  REQUIRE(fixture->parser().expect(TokenKind::IntLiteral));
  REQUIRE(fixture->current().kind == TokenKind::EoF);
}

TEST_CASE("Parser: Complex token sequence", "[parser][buffer]") {
  auto fixture = createParserFixture("true null 123 \"test\" identifier false");

  // Test sequence of different token types
  std::vector<TokenKind> expectedSequence = {
      TokenKind::True,       TokenKind::Null,
      TokenKind::IntLiteral, TokenKind::StringLiteral,
      TokenKind::Ident,      TokenKind::False,
      TokenKind::EoF};

  for (TokenKind expectedKind : expectedSequence) {
    REQUIRE(fixture->current().kind == expectedKind);
    if (expectedKind != TokenKind::EoF) {
      fixture->advance();
    }
  }

  REQUIRE(fixture->isAtEnd());
}

TEST_CASE("Parser: Buffer state after parsing error", "[parser][buffer]") {
  auto fixture = createParserFixture("42 unexpected_but_valid_token");

  // Current should be integer
  REQUIRE(fixture->current().kind == TokenKind::IntLiteral);

  // Try to expect wrong token (should fail but not corrupt buffer)
  REQUIRE_FALSE(fixture->parser().expect(TokenKind::StringLiteral));

  // Buffer should be unchanged
  REQUIRE(fixture->current().kind == TokenKind::IntLiteral);
  REQUIRE(fixture->lookahead(1).kind == TokenKind::Ident);

  // Should still be able to advance normally
  fixture->advance();
  REQUIRE(fixture->current().kind == TokenKind::Ident);
}
