#include "catch2.hpp"
#include "cxy/memory/arena.hpp"
#include "cxy/diagnostics.hpp"
#include "cxy/lexer.hpp"
#include "cxy/strings.hpp"
#include "cxy/token.hpp"
#include "lexer_test_helper.hpp"

#include <iostream>
#include <memory>

using namespace cxy;

// Phase 7: Multi-Buffer Management Tests

TEST_CASE("Basic buffer pushing and popping", "[lexer][phase7]") {
  LexerTestHelper helper;

  std::string mainContent = "var main = 1;";
  ArenaAllocator arena(1024 * 1024);
  StringInterner interner(arena);
  Lexer lexer("main.cxy", mainContent, helper.getLogger(), interner);

  // Should be able to push a new buffer
  bool success = lexer.pushBuffer("other.cxy", "var other = 2;");
  CHECK(success);

  // Should be able to push another buffer
  bool success2 = lexer.pushBuffer("third.cxy", "var third = 3;");
  CHECK(success2);

  // Buffers will pop automatically when their content is exhausted

  // popBuffer is now private - buffers pop automatically on EOF

  CHECK_FALSE(helper.hasErrors());
}

TEST_CASE("Cycle detection prevents infinite includes", "[lexer][phase7]") {
  LexerTestHelper helper;

  std::string content = "var main = 1;";
  ArenaAllocator arena(1024 * 1024);
  StringInterner interner(arena);
  Lexer lexer("main.cxy", content, helper.getLogger(), interner);

  // Should be able to include different file
  bool success1 = lexer.pushBuffer("utils.cxy", "var util = 2;");
  CHECK(success1);

  // Should be able to include another different file
  bool success2 = lexer.pushBuffer("helpers.cxy", "var helper = 3;");
  CHECK(success2);

  // Should detect cycle when trying to include main.cxy again
  bool success3 = lexer.pushBuffer("main.cxy", "var other = 4;");
  CHECK_FALSE(success3);

  // Should also detect cycle for utils.cxy
  bool success4 = lexer.pushBuffer("utils.cxy", "var other_util = 5;");
  CHECK_FALSE(success4);

  // Should still be able to include a new file
  bool success5 = lexer.pushBuffer("new_file.cxy", "var new_var = 6;");
  CHECK(success5);
}

TEST_CASE("Token stream with include directive simulation", "[lexer][phase7]") {
  LexerTestHelper helper;

  // main.cxy content: var a = 10; var b = include "utils.cxy"; println(b);
  std::string mainContent = R"(var a = 10;
var b = include "utils.cxy";
println(b);)";

  // utils.cxy content: a + 10
  std::string utilsContent = "a + 10";

  auto &logger = helper.getLogger();
  ArenaAllocator arena(1024 * 1024);
  StringInterner interner(arena);
  Lexer lexer("main.cxy", mainContent, logger, interner);

  // Register sources with helper's source manager for token text extraction
  helper.tokenize(mainContent, "main.cxy");
  helper.tokenize(utilsContent, "utils.cxy");

  std::vector<Token> tokens;

  // Read tokens from main.cxy until we hit the include
  Token token;

  // var a = 10;
  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Var);
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Ident);
  CHECK(helper.getTokenText(token) == "a");
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Assign);
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::IntLiteral);
  CHECK(helper.getTokenText(token) == "10");
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Semicolon);
  tokens.push_back(token);

  // var b = include "utils.cxy";
  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Var);
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Ident);
  CHECK(helper.getTokenText(token) == "b");
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Assign);
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Include);
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::StringLiteral);
  CHECK(helper.getTokenText(token) == "\"utils.cxy\"");
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Semicolon);
  tokens.push_back(token);

  // Now simulate pushing the utils.cxy buffer
  bool success = lexer.pushBuffer("utils.cxy", utilsContent);
  REQUIRE(success);

  // Read tokens from utils.cxy: a + 10
  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Ident);
  CHECK(helper.getTokenText(token) == "a");
  CHECK(token.location.filename == "utils.cxy");
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Plus);
  CHECK(token.location.filename == "utils.cxy");
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::IntLiteral);
  CHECK(helper.getTokenText(token) == "10");
  CHECK(token.location.filename == "utils.cxy");
  tokens.push_back(token);

  // After utils.cxy EOF, lexer should automatically pop back to main.cxy
  // Continue with: println(b);
  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Ident);
  CHECK(helper.getTokenText(token) == "println");
  CHECK(token.location.filename == "main.cxy");
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::LParen);
  CHECK(token.location.filename == "main.cxy");
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Ident);
  CHECK(helper.getTokenText(token) == "b");
  CHECK(token.location.filename == "main.cxy");
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::RParen);
  CHECK(token.location.filename == "main.cxy");
  tokens.push_back(token);

  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Semicolon);
  CHECK(token.location.filename == "main.cxy");
  tokens.push_back(token);

  // Finally EOF
  token = lexer.nextToken();
  CHECK(token.kind == TokenKind::EoF);
  tokens.push_back(token);
}

TEST_CASE("Buffer popping happens automatically on EOF", "[lexer][phase7]") {
  LexerTestHelper helper;

  std::string mainContent = "var main = 1;";
  auto &logger = helper.getLogger();
  ArenaAllocator arena(1024 * 1024);
  StringInterner interner(arena);
  Lexer lexer("main.cxy", mainContent, logger, interner);

  // Push a short buffer
  bool success = lexer.pushBuffer("short.cxy", "var x;");
  REQUIRE(success);

  // Register sources with helper's source manager
  helper.tokenize(mainContent, "main.cxy");
  helper.tokenize("var x;", "short.cxy");

  // Read through the short buffer
  Token token;

  // Read tokens: var, x, ;, then should auto-pop to main buffer
  token = lexer.nextToken(); // var
  CHECK(token.kind == TokenKind::Var);

  token = lexer.nextToken(); // x
  CHECK(token.kind == TokenKind::Ident);

  token = lexer.nextToken(); // ;
  CHECK(token.kind == TokenKind::Semicolon);

  // Next token should come from main buffer (auto-popped)
  token = lexer.nextToken(); // var (from main)
  CHECK(token.kind == TokenKind::Var);

  token = lexer.nextToken(); // main
  CHECK(token.kind == TokenKind::Ident);
  auto text = helper.getTokenText(token);
  CHECK(text == "main");
}

TEST_CASE("Nested buffer management", "[lexer][phase7]") {
  LexerTestHelper helper;

  std::string mainContent = "var a = 1;";
  ArenaAllocator arena(1024 * 1024);
  StringInterner interner(arena);
  Lexer lexer("main.cxy", mainContent, helper.getLogger(), interner);

  // Create a chain: main -> level1 -> level2 -> level3
  bool s1 = lexer.pushBuffer("level1.cxy", "var b = 2;");
  REQUIRE(s1);

  bool s2 = lexer.pushBuffer("level2.cxy", "var c = 3;");
  REQUIRE(s2);

  bool s3 = lexer.pushBuffer("level3.cxy", "var d = 4;");
  REQUIRE(s3);

  // Should detect cycle if trying to include any file in the chain
  CHECK_FALSE(lexer.pushBuffer("main.cxy", "var x;"));
  CHECK_FALSE(lexer.pushBuffer("level1.cxy", "var y;"));
  CHECK_FALSE(lexer.pushBuffer("level2.cxy", "var z;"));
  CHECK_FALSE(lexer.pushBuffer("level3.cxy", "var w;"));

  // Should allow new files not in the chain
  CHECK(lexer.pushBuffer("other.cxy", "var other;"));

  // Buffers will pop automatically when their content is exhausted
  // We can verify this by reading all tokens until EOF
  Token token;
  do {
    token = lexer.nextToken();
  } while (token.kind != TokenKind::EoF);
}

TEST_CASE("Empty buffer handling", "[lexer][phase7]") {
  LexerTestHelper helper;

  std::string mainContent = "var main = 1;";
  ArenaAllocator arena(1024 * 1024);
  StringInterner interner(arena);
  Lexer lexer("main.cxy", mainContent, helper.getLogger(), interner);

  // Push empty buffer
  bool success = lexer.pushBuffer("empty.cxy", "");
  REQUIRE(success);

  // Should immediately pop to main buffer on first nextToken call
  Token token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Var); // from main buffer
}

TEST_CASE("Whitespace-only buffer handling", "[lexer][phase7]") {
  LexerTestHelper helper;

  std::string mainContent = "var main = 1;";
  ArenaAllocator arena(1024 * 1024);
  StringInterner interner(arena);
  Lexer lexer("main.cxy", mainContent, helper.getLogger(), interner);

  // Push whitespace-only buffer
  bool success = lexer.pushBuffer("whitespace.cxy", "   \n\t  \n  ");
  REQUIRE(success);

  // Should skip whitespace and pop to main buffer
  Token token = lexer.nextToken();
  CHECK(token.kind == TokenKind::Var); // from main buffer
}

TEST_CASE("Buffer stack location tracking", "[lexer][phase7]") {
  LexerTestHelper helper;

  std::string mainContent = "var main = 1;";
  ArenaAllocator arena(1024 * 1024);
  StringInterner interner(arena);
  Lexer lexer("main.cxy", mainContent, helper.getLogger(), interner);

  bool success = lexer.pushBuffer("other.cxy", "var other = 2;");
  REQUIRE(success);

  // First token should come from other.cxy
  Token token = lexer.nextToken(); // var
  CHECK(token.location.filename == "other.cxy");
  CHECK(token.location.start.row == 1);

  token = lexer.nextToken(); // other
  CHECK(token.location.filename == "other.cxy");

  token = lexer.nextToken(); // =
  CHECK(token.location.filename == "other.cxy");

  token = lexer.nextToken(); // 2
  CHECK(token.location.filename == "other.cxy");

  token = lexer.nextToken(); // ;
  CHECK(token.location.filename == "other.cxy");

  // After EOF of other.cxy, should continue with main.cxy
  token = lexer.nextToken(); // var (from main)
  CHECK(token.location.filename == "main.cxy");
}
