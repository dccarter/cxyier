#include "cxy/token.hpp"

namespace cxy {

std::string_view readTokenText(const Token &token,
                               const SourceManager &sourceManager) {
  // For fixed tokens (symbols and keywords), return the known string
  if (isSymbol(token.kind) || isKeyword(token.kind)) {
    return tokenKindToString(token.kind);
  }

  // For special tokens, read from source using location
  switch (token.kind) {
  case TokenKind::Ident:
  case TokenKind::IntLiteral:
  case TokenKind::FloatLiteral:
  case TokenKind::CharLiteral:
  case TokenKind::StringLiteral:
  case TokenKind::LString:
  case TokenKind::RString:
    // Read the text from the source using the token's location
    return sourceManager.getRangeView(token.location);

  case TokenKind::EoF:
    return "";

  case TokenKind::Error:
  default:
    // For error tokens or unknown tokens, try to read from source
    return sourceManager.getRangeView(token.location);
  }
}

InternedString getTokenValue(const Token &token,
                             const SourceManager &sourceManager,
                             StringInterner &interner) {
  // Get the token text
  std::string_view text = readTokenText(token, sourceManager);

  // Intern if this token type benefits from it
  if (shouldInternTokenText(token.kind)) {
    return interner.intern(text);
  }

  // For non-interned tokens, create a temporary InternedString
  // Note: This might not be the best approach - consider returning
  // a variant<InternedString, string_view> or similar
  return interner.intern(text);
}

std::string_view getTokenText(const Token &token,
                              const SourceManager &sourceManager) {
  return readTokenText(token, sourceManager);
}

} // namespace cxy
