#pragma once

#include "cxy/diagnostics.hpp"
#include "cxy/strings.hpp"
#include <cstdint>
#include <format>
#include <string>
#include <string_view>

namespace cxy {

/**
 * @brief Enumeration for integer literal types based on suffixes.
 */
enum class IntegerType : uint8_t {
  Auto, // No suffix, infer from value
  I8,   // Explicit i8 suffix
  I16,  // Explicit i16 suffix
  I32,  // Explicit i32 suffix
  I64,  // Explicit i64 suffix
  U8,   // Explicit u8 suffix
  U16,  // Explicit u16 suffix
  U32,  // u suffix (or explicit u32)
  U64,  // ul/ull suffix (or explicit u64)
  I128, // Explicit i128 suffix
  U128  // Explicit u128 suffix
};

/**
 * @brief Enumeration for floating-point literal types based on suffixes.
 */
enum class FloatType : uint8_t {
  Auto, // No suffix, defaults to f64
  F32,  // f suffix
  F64   // d suffix or no suffix (default)
};

/**
 * @brief Macro defining all symbol tokens in the Cxy language.
 *
 * These are punctuation and operator symbols that have special meaning
 * in the language syntax.
 */
#define SYMBOL_LIST(f)                                                         \
  f(LParen, "(") f(RParen, ")") f(LBracket, "[") f(RBracket, "]")              \
      f(LBrace, "{") f(RBrace, "}") f(At, "@") f(Hash, "#") f(LNot, "!") f(    \
          BNot, "~") f(Dot, ".") f(DotDot, "..") f(Elipsis, "...")             \
          f(Question, "?") f(Comma, ",") f(Colon, ":") f(Semicolon, ";") f(    \
              Assign, "=") f(Equal, "==") f(NotEqual, "!=") f(FatArrow, "=>")  \
              f(ThinArrow, "->") f(Less, "<") f(LessEqual, "<=") f(Shl, "<<")  \
                  f(ShlEqual, "<<=") f(Greater, ">") f(GreaterEqual, ">=") f(  \
                      Shr, ">>") f(ShrEqual,                                   \
                                   ">>=") f(Plus, "+") f(Minus, "-")           \
                      f(Mult, "*") f(Div, "/") f(Mod, "%") f(BAnd, "&")        \
                          f(BXor, "^") f(BOr, "|") f(LAnd, "&&") f(LOr, "||")  \
                              f(PlusPlus, "++") f(MinusMinus, "--") f(         \
                                  PlusEqual, "+=") f(MinusEqual, "-=")         \
                                  f(MultEqual, "*=") f(DivEqual,               \
                                                       "/=") f(ModEqual, "%=") \
                                      f(BAndEqual, "&=") f(BAndDot, "&.") f(   \
                                          BXorEqual, "^=") f(BOrEqual, "|=")   \
                                          f(Quote, "`") f(CallOverride, "()")  \
                                              f(IndexOverride,                 \
                                                "[]") f(IndexAssignOvd, "[]=") \
                                                  f(AstMacroAccess, "#.")      \
                                                      f(Define, "##")          \
                                                          f(BangColon, "!:")

/**
 * @brief Macro defining primitive type keywords.
 *
 * Note: PRIM_TYPE_LIST is referenced but not defined in the source.
 * This is a placeholder that should be defined based on the language's
 * primitive types (e.g., i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, etc.)
 */
#define PRIM_TYPE_LIST(f)                                                      \
  f(I8, "i8") f(I16, "i16") f(I32, "i32") f(I64, "i64") f(I128, "i128")        \
      f(U8, "u8") f(U16, "u16") f(U32, "u32") f(U64, "u64") f(U128, "u128")    \
          f(F32, "f32") f(F64, "f64") f(Bool, "bool") f(Char, "char")

/**
 * @brief Macro defining all keyword tokens in the Cxy language.
 *
 * These are reserved words that have special meaning in the language.
 */
#define KEYWORD_LIST(f)                                                        \
  f(Virtual, "virtual") f(Auto, "auto") f(True, "true") f(False, "false") f(   \
      Null, "null") f(If, "if") f(Else, "else") f(Match, "match")              \
      f(For, "for") f(In, "in") f(Is, "is") f(While, "while") f(               \
          Break, "break") f(Return, "return") f(Yield, "yield") f(Continue,    \
                                                                  "continue")  \
          f(Func, "func") f(Var, "var") f(Const, "const") f(Type, "type") f(   \
              Native, "native") f(Extern, "extern") f(Exception, "exception")  \
              f(Struct, "struct") f(Enum, "enum") f(Pub, "pub")                \
                  f(Opaque, "opaque") f(Catch, "catch") f(Raise, "raise") f(   \
                      Async,                                                   \
                      "async") f(Launch, "launch") f(Ptrof, "ptrof")           \
                      f(Await, "await") f(Delete, "delete") f(                 \
                          Discard,                                             \
                          "discard") f(Switch,                                 \
                                       "switch") f(Case,                       \
                                                   "case") f(Default,          \
                                                             "default")        \
                          f(Defer, "defer") f(                                 \
                              Macro,                                           \
                              "macro") f(Void,                                 \
                                         "void") f(String, "string")           \
                              f(Range, "range") f(Module, "module") f(         \
                                  Import,                                      \
                                  "import") f(Include, "include")              \
                                  f(CSources,                                  \
                                    "cSources") f(As, "as")                    \
                                      f(Asm, "asm") f(From, "from") f(         \
                                          Unsafe,                              \
                                          "unsafe") f(Interface, "interface")  \
                                          f(This,                              \
                                            "this") f(ThisClass, "This")       \
                                              f(Super,                         \
                                                "super") f(Class, "class")     \
                                                  f(Defined,                   \
                                                    "defined") f(Test, "test") \
                                                      f(Plugin, "plugin")      \
                                                          f(CBuild, "__cc")    \
                                                              PRIM_TYPE_LIST(  \
                                                                  f)

/**
 * @brief Macro defining special token types.
 *
 * These are tokens that don't have a fixed string representation
 * but represent categories of tokens (identifiers, literals, etc.)
 */
#define SPECIAL_TOKEN_LIST(f)                                                  \
  f(Ident, "identifier") f(IntLiteral, "integer literal")                      \
      f(FloatLiteral, "floating-point literal")                                \
          f(CharLiteral, "character literal")                                  \
              f(StringLiteral, "string literal") f(LString, "`(")              \
                  f(RString, ")`") f(EoF, "end of file")                       \
                      f(Error, "invalid token")

/**
 * @brief Master macro defining all token types.
 *
 * This combines all the individual token type macros into one
 * comprehensive list for generating the TokenKind enum and
 * related functionality.
 */
#define TOKEN_LIST(f)                                                          \
  SYMBOL_LIST(f)                                                               \
  KEYWORD_LIST(f)                                                              \
  SPECIAL_TOKEN_LIST(f)

/**
 * @brief Enumeration of all token kinds in the Cxy language.
 *
 * This enum is automatically generated from the TOKEN_LIST macro
 * and represents every possible token type that the lexer can produce.
 */
enum class TokenKind : uint32_t {
#define TOKEN_ENUM(name, str) name,
  TOKEN_LIST(TOKEN_ENUM)
#undef TOKEN_ENUM

  // Special values for range checking
  FirstSymbol = LParen,
  LastSymbol = BangColon,
  FirstKeyword = Virtual,
  LastKeyword = CBuild, // Note: This assumes CBuild is the last in KEYWORD_LIST
  FirstSpecial = Ident,
  LastSpecial = Error
};

/**
 * @brief Convert a TokenKind to its string representation.
 *
 * For symbols and keywords, this returns the actual token text.
 * For special tokens, this returns a descriptive name.
 *
 * @param kind The token kind to convert
 * @return String view containing the token representation
 *
 * Example:
 *   tokenKindToString(TokenKind::LParen) returns "("
 *   tokenKindToString(TokenKind::If) returns "if"
 *   tokenKindToString(TokenKind::Ident) returns "identifier"
 */
constexpr std::string_view tokenKindToString(TokenKind kind) {
  switch (kind) {
#define TOKEN_STRING(name, str)                                                \
  case TokenKind::name:                                                        \
    return str;
    TOKEN_LIST(TOKEN_STRING)
#undef TOKEN_STRING
  default:
    return "unknown";
  }
}

/**
 * @brief Convert a TokenKind to its enum name.
 *
 * This returns the actual enum name (e.g., "LParen" instead of "(").
 * Useful for debugging and code generation.
 *
 * @param kind The token kind to convert
 * @return String view containing the enum name
 */
constexpr std::string_view tokenKindToEnumName(TokenKind kind) {
  switch (kind) {
#define TOKEN_ENUM_NAME(name, str)                                             \
  case TokenKind::name:                                                        \
    return #name;
    TOKEN_LIST(TOKEN_ENUM_NAME)
#undef TOKEN_ENUM_NAME
  default:
    return "unknown";
  }
}

/**
 * @brief Check if a token kind is a symbol (punctuation/operator).
 */
constexpr bool isSymbol(TokenKind kind) {
  return kind >= TokenKind::FirstSymbol && kind <= TokenKind::LastSymbol;
}

/**
 * @brief Check if a token kind is a keyword.
 */
constexpr bool isKeyword(TokenKind kind) {
  return kind >= TokenKind::FirstKeyword && kind <= TokenKind::LastKeyword;
}

/**
 * @brief Check if a token kind is a special token (identifier, literal, etc.).
 */
constexpr bool isSpecial(TokenKind kind) {
  return kind >= TokenKind::FirstSpecial && kind <= TokenKind::LastSpecial;
}

/**
 * @brief Check if a token kind represents a literal value.
 */
constexpr bool isLiteral(TokenKind kind) {
  return kind == TokenKind::IntLiteral || kind == TokenKind::FloatLiteral ||
         kind == TokenKind::CharLiteral || kind == TokenKind::StringLiteral ||
         kind == TokenKind::True || kind == TokenKind::False ||
         kind == TokenKind::Null;
}

/**
 * @brief Check if a token kind is a primitive type.
 */
constexpr bool isPrimitiveType(TokenKind kind) {
  return (kind >= TokenKind::I8 && kind <= TokenKind::Char) ||
         kind == TokenKind::Void || kind == TokenKind::String ||
         kind == TokenKind::Bool;
}

/**
 * @brief Check if a token kind represents a binary operator.
 */
constexpr bool isBinaryOperator(TokenKind kind) {
  return kind == TokenKind::Plus || kind == TokenKind::Minus ||
         kind == TokenKind::Mult || kind == TokenKind::Div ||
         kind == TokenKind::Mod || kind == TokenKind::Equal ||
         kind == TokenKind::NotEqual || kind == TokenKind::Less ||
         kind == TokenKind::LessEqual || kind == TokenKind::Greater ||
         kind == TokenKind::GreaterEqual || kind == TokenKind::LAnd ||
         kind == TokenKind::LOr || kind == TokenKind::BAnd ||
         kind == TokenKind::BOr || kind == TokenKind::BXor ||
         kind == TokenKind::Shl || kind == TokenKind::Shr;
}

/**
 * @brief Check if a token kind represents a unary operator.
 */
constexpr bool isUnaryOperator(TokenKind kind) {
  return kind == TokenKind::LNot || kind == TokenKind::BNot ||
         kind == TokenKind::Plus || kind == TokenKind::Minus ||
         kind == TokenKind::PlusPlus || kind == TokenKind::MinusMinus;
}

/**
 * @brief Check if a token kind represents an assignment operator.
 */
constexpr bool isAssignmentOperator(TokenKind kind) {
  return kind == TokenKind::Assign || kind == TokenKind::PlusEqual ||
         kind == TokenKind::MinusEqual || kind == TokenKind::MultEqual ||
         kind == TokenKind::DivEqual || kind == TokenKind::ModEqual ||
         kind == TokenKind::BAndEqual || kind == TokenKind::BXorEqual ||
         kind == TokenKind::BOrEqual || kind == TokenKind::ShlEqual ||
         kind == TokenKind::ShrEqual;
}

/**
 * @brief Represents a single token in the Cxy language.
 *
 * A token contains the token type and its source location. The actual text
 * can be retrieved from the source using the location information.
 * For literal tokens, parsed values are stored in the value union.
 */
struct Token {
  TokenKind kind;    ///< The type of this token
  Location location; ///< Source location where this token was found

  /**
   * @brief Union holding parsed literal values for different token types.
   */
  union Value {
    bool boolValue;     ///< For true/false literals
    uint32_t charValue; ///< For character literals (Unicode codepoint)

    struct {
      __uint128_t value;
      IntegerType type;
    } intValue; ///< For integer literals with type info

    struct {
      double value;
      FloatType type;
    } floatValue; ///< For floating-point literals with type info

    /**
     * @brief Default constructor initializes to zero.
     */
    Value() : intValue{0, IntegerType::Auto} {}

    /**
     * @brief Constructor for boolean values.
     */
    explicit Value(bool b) : boolValue(b) {}

    /**
     * @brief Constructor for character values.
     */
    explicit Value(uint32_t c) : charValue(c) {}

    /**
     * @brief Constructor for integer values with type.
     */
    explicit Value(__uint128_t i, IntegerType t = IntegerType::Auto)
        : intValue{i, t} {}

    /**
     * @brief Constructor for floating-point values with type.
     */
    explicit Value(double f, FloatType t = FloatType::Auto)
        : floatValue{f, t} {}
  } value;

  bool hasValue = false; ///< Whether the value union contains valid data

  /**
   * @brief Default constructor creates an invalid/error token.
   */
  Token() : kind(TokenKind::Error), location(), value(), hasValue(false) {}

  /**
   * @brief Construct a token with kind and location.
   *
   * @param k The token kind
   * @param loc The source location
   */
  Token(TokenKind k, Location loc)
      : kind(k), location(loc), value(), hasValue(false) {}

  /**
   * @brief Construct a token with a boolean value.
   *
   * @param k The token kind (should be True or False)
   * @param loc The source location
   * @param val The boolean value
   */
  Token(TokenKind k, Location loc, bool val)
      : kind(k), location(loc), value(val), hasValue(true) {}

  /**
   * @brief Construct a token with a character value.
   *
   * @param k The token kind (should be CharLiteral)
   * @param loc The source location
   * @param val The Unicode codepoint value
   */
  Token(TokenKind k, Location loc, uint32_t val)
      : kind(k), location(loc), value(val), hasValue(true) {}

  /**
   * @brief Construct a token with an integer value and type.
   *
   * @param k The token kind (should be IntLiteral)
   * @param loc The source location
   * @param val The integer value
   * @param type The integer type from suffix
   */
  Token(TokenKind k, Location loc, __uint128_t val,
        IntegerType type = IntegerType::Auto)
      : kind(k), location(loc), value(val, type), hasValue(true) {}

  /**
   * @brief Construct a token with a floating-point value and type.
   *
   * @param k The token kind (should be FloatLiteral)
   * @param loc The source location
   * @param val The floating-point value
   * @param type The float type from suffix
   */
  Token(TokenKind k, Location loc, double val, FloatType type = FloatType::Auto)
      : kind(k), location(loc), value(val, type), hasValue(true) {}

  /**
   * @brief Check if this is a valid token.
   * @return false if this is an error token
   */
  bool isValid() const { return kind != TokenKind::Error; }

  /**
   * @brief Check if this token is of a specific kind.
   * @param k The kind to check against
   * @return true if the token matches the specified kind
   */
  bool is(TokenKind k) const { return kind == k; }

  /**
   * @brief Check if this token is one of several kinds.
   * @param k1 First kind to check
   * @param k2 Second kind to check
   * @param kinds Additional kinds to check
   * @return true if the token matches any of the specified kinds
   */
  template <typename... TokenKinds>
  bool isOneOf(TokenKind k1, TokenKind k2, TokenKinds... kinds) const {
    return is(k1) || isOneOf(k2, kinds...);
  }

  /**
   * @brief Base case for isOneOf template recursion.
   */
  bool isOneOf(TokenKind k) const { return is(k); }

  /**
   * @brief Check if this token represents an end-of-file.
   */
  bool isEof() const { return kind == TokenKind::EoF; }

  /**
   * @brief Check if this token has a parsed value.
   */
  bool hasLiteralValue() const { return hasValue; }

  /**
   * @brief Get the boolean value (for True/False tokens).
   * @return The boolean value, or false if not applicable
   */
  bool getBoolValue() const {
    return hasValue && (kind == TokenKind::True || kind == TokenKind::False)
               ? value.boolValue
               : false;
  }

  /**
   * @brief Get the character value (for CharLiteral tokens).
   * @return The Unicode codepoint, or 0 if not applicable
   */
  uint32_t getCharValue() const {
    return hasValue && kind == TokenKind::CharLiteral ? value.charValue : 0;
  }

  /**
   * @brief Get the integer value (for IntLiteral tokens).
   * @return The integer value, or 0 if not applicable
   */
  __uint128_t getIntValue() const {
    return hasValue && kind == TokenKind::IntLiteral ? value.intValue.value : 0;
  }

  /**
   * @brief Get the integer type (for IntLiteral tokens).
   * @return The integer type from suffix, or Auto if not applicable
   */
  IntegerType getIntType() const {
    return hasValue && kind == TokenKind::IntLiteral ? value.intValue.type
                                                     : IntegerType::Auto;
  }

  /**
   * @brief Get the floating-point value (for FloatLiteral tokens).
   * @return The floating-point value, or 0.0 if not applicable
   */
  double getFloatValue() const {
    return hasValue && kind == TokenKind::FloatLiteral ? value.floatValue.value
                                                       : 0.0;
  }

  /**
   * @brief Get the floating-point type (for FloatLiteral tokens).
   * @return The float type from suffix, or Auto if not applicable
   */
  FloatType getFloatType() const {
    return hasValue && kind == TokenKind::FloatLiteral ? value.floatValue.type
                                                       : FloatType::Auto;
  }

  /**
   * @brief Equality comparison operator.
   */
  bool operator==(const Token &other) const {
    if (kind != other.kind || location != other.location) {
      return false;
    }

    // If both have values, compare them based on token kind
    if (hasValue && other.hasValue) {
      switch (kind) {
      case TokenKind::True:
      case TokenKind::False:
        return value.boolValue == other.value.boolValue;
      case TokenKind::CharLiteral:
        return value.charValue == other.value.charValue;
      case TokenKind::IntLiteral:
        return value.intValue.value == other.value.intValue.value &&
               value.intValue.type == other.value.intValue.type;
      case TokenKind::FloatLiteral:
        return value.floatValue.value == other.value.floatValue.value &&
               value.floatValue.type == other.value.floatValue.type;
      default:
        return true; // Same kind and location is enough for non-literal tokens
      }
    }

    // If hasValue flags differ, tokens are different
    return hasValue == other.hasValue;
  }
};

/**
 * @brief Check if a token kind should have its text interned.
 *
 * Identifiers, keywords, and string literals are interned for memory efficiency
 * and fast comparison.
 */
const constexpr bool shouldInternTokenText(TokenKind kind) {
  return kind == TokenKind::Ident || kind == TokenKind::StringLiteral ||
         isKeyword(kind);
}

/**
 * @brief Read the text value of a token from the source.
 *
 * For fixed tokens (symbols, keywords), this returns the known string.
 * For variable tokens (identifiers, literals), this reads from the source.
 *
 * @param token The token to read
 * @param sourceManager Source manager to read from
 * @return String view of the token text
 */
std::string_view readTokenText(const Token &token,
                               const SourceManager &sourceManager);

/**
 * @brief Get the text value of a token, interning it if appropriate.
 *
 * This function determines the token's text value and interns it if the token
 * type benefits from interning (identifiers, keywords, string literals).
 *
 * @param token The token to get text for
 * @param sourceManager Source manager to read from
 * @param interner String interner for caching
 * @return Interned string handle, or non-interned string view
 */
InternedString getTokenValue(const Token &token,
                             const SourceManager &sourceManager,
                             StringInterner &interner);

/**
 * @brief Get the text value of a token without interning.
 *
 * This is useful when you need the raw text but don't want to intern it.
 *
 * @param token The token to get text for
 * @param sourceManager Source manager to read from
 * @return String view of the token text
 */
std::string_view getTokenText(const Token &token,
                              const SourceManager &sourceManager);

} // namespace cxy

// std::formatter specialization for Token to enable std::format support
template <> struct std::formatter<cxy::Token> {
  constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }

  auto format(const cxy::Token &token, std::format_context &ctx) const {
    return std::format_to(ctx.out(), "{}@{}",
                          cxy::tokenKindToEnumName(token.kind), token.location);
  }
};
