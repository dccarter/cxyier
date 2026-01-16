#pragma once

#include <format>
#include <string_view>

namespace cxy::ast {

// X-Macro for all AST node types
// clang-format off
#define CXY_AST_NODES(f) \
    /* Special/Utility Nodes */ \
    f(Noop)                     \
                                \
    /* Literals */              \
    f(Bool)                     \
    f(Int)                      \
    f(Float)                    \
    f(String)                   \
    f(Char)                     \
    f(Null)                     \
                                \
    /* Identifiers and Paths */ \
    f(Identifier)               \
    f(QualifiedPath)            \
                                \
    /* Types */                 \
    f(PrimitiveType)            \
                                \
    /* Attributes */            \
    f(Attribute)                \
    f(AttributeList)            \
                                \
    /* Expressions */           \
    f(UnaryExpr)                \
    f(BinaryExpr)               \
    f(TernaryExpr)              \
    f(AssignmentExpr)           \
    f(GroupExpr)                \
    f(StmtExpr)                 \
    f(StringExpr)               \
    f(CastExpr)                 \
    f(CallExpr)                 \
    f(IndexExpr)                \
    f(ArrayExpr)                \
    f(TupleExpr)                \
    f(FieldExpr)                \
    f(StructExpr)               \
    f(MemberExpr)               \
    f(MacroCallExpr)            \
    f(ClosureExpr)              \
    f(RangeExpr)                \
    f(SpreadExpr)               \
                                \
    /* Statements */            \
    f(ExprStmt)                 \
    f(BlockStmt)                \
    f(BreakStmt)                \
    f(ContinueStmt)             \
    f(DeferStmt)                \
    f(ReturnStmt)               \
    f(YieldStmt)                \
    f(IfStmt)                   \
    f(ForStmt)                  \
    f(WhileStmt)                \
    f(SwitchStmt)               \
    f(MatchStmt)                \
    f(CaseStmt)
// clang-format on

// Generate the enum with ast prefix
enum NodeKind : unsigned char {
#define CXY_AST_NODE(name) ast##name,
  CXY_AST_NODES(CXY_AST_NODE)
#undef CXY_AST_NODE

  // Special values
  astCount,
  astInvalid = 255
};

// Convert NodeKind to string without "ast" prefix
constexpr std::string_view nodeKindToString(NodeKind kind) {
  switch (kind) {
#define CXY_AST_NODE(name)                                                     \
  case ast##name:                                                              \
    return #name;
    CXY_AST_NODES(CXY_AST_NODE)
#undef CXY_AST_NODE
  case astInvalid:
    return "Invalid";
  default:
    return "Unknown";
  }
}

} // namespace cxy::ast

// std::format specialization
template <> struct std::formatter<cxy::ast::NodeKind> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  auto format(cxy::ast::NodeKind kind, format_context &ctx) const {
    return format_to(ctx.out(), "{}", cxy::ast::nodeKindToString(kind));
  }
};
