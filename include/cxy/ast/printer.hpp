#pragma once

#include "cxy/ast/visitor.hpp"
#include "cxy/memory.hpp"
#include <functional>
#include <iostream>
#include <sstream>

namespace cxy::ast {

/**
 * @brief Configuration flags for AST printer output.
 */
enum class PrinterFlags : uint32_t {
  None = 0,
  IncludeLocation = 1 << 0, ///< Include source location info
  IncludeTypes = 1 << 1,    ///< Include type annotations
  IncludeFlags = 1 << 2,    ///< Include node flags
  IncludeMetadata = 1 << 3, ///< Include metadata
  CompactLiterals = 1 << 4, ///< Compact literal representation
  CompactMode = 1 << 5,     ///< Minimize whitespace
  Default = IncludeLocation
};

// Enable bitwise operations for PrinterFlags
constexpr PrinterFlags operator|(PrinterFlags a, PrinterFlags b) {
  return static_cast<PrinterFlags>(static_cast<uint32_t>(a) |
                                   static_cast<uint32_t>(b));
}

constexpr PrinterFlags operator&(PrinterFlags a, PrinterFlags b) {
  return static_cast<PrinterFlags>(static_cast<uint32_t>(a) &
                                   static_cast<uint32_t>(b));
}

constexpr bool operator!(PrinterFlags flags) {
  return static_cast<uint32_t>(flags) == 0;
}

/**
 * @brief Configuration structure for AST printer.
 */
struct PrinterConfig {
  PrinterFlags flags = PrinterFlags::Default;
  uint32_t maxDepth = 0;       ///< 0 = unlimited
  uint32_t indentSize = 2;     ///< Spaces per indent level
  std::string_view nodePrefix; ///< Optional prefix for node names
  std::function<bool(const ASTNode *)> nodeFilter; ///< Optional node filter

  /**
   * @brief Check if a flag is set.
   */
  bool hasFlag(PrinterFlags flag) const { return !!(flags & flag); }
};

/**
 * @brief AST printer that generates S-expression format output.
 *
 * The AST printer traverses an AST using the visitor pattern and generates
 * readable S-expression output. It supports various configuration options
 * to control the level of detail and formatting.
 */
class ASTPrinter : public ConstASTVisitor {
public:
  explicit ASTPrinter(const PrinterConfig &config = {});
  explicit ASTPrinter(ArenaAllocator &arena, const PrinterConfig &config = {});

  // Main printing interface
  std::string print(const ASTNode *root);
  void print(const ASTNode *root, std::ostream &out);
  void printToStream(const ASTNode *root, std::ostream &out);

  // Configuration
  void setConfig(const PrinterConfig &config);
  const PrinterConfig &getConfig() const { return config_; }

  // Statistics and debugging
  uint32_t getNodesVisited() const { return nodesVisited_; }
  uint32_t getMaxDepthReached() const { return maxDepthReached_; }

public:
  // Override main visit method for statistics and depth tracking
  void visit(const ASTNode *node) override;

protected:
  // ConstASTVisitor implementations
  bool visitNode(const ASTNode *node) override;
  void visitNodePost(const ASTNode *node) override;

  // Specific node visitors
  bool visitNoop(const ASTNode *node) override;

  bool visitBool(const BoolLiteralNode *node) override;
  bool visitInt(const IntLiteralNode *node) override;
  bool visitFloat(const FloatLiteralNode *node) override;
  bool visitString(const StringLiteralNode *node) override;
  bool visitChar(const CharLiteralNode *node) override;
  bool visitNull(const NullLiteralNode *node) override;

  bool visitIdentifier(const IdentifierNode *node) override;
  bool visitQualifiedPath(const QualifiedPathNode *node) override;

  bool visitUnary(const UnaryExpressionNode *node) override;
  bool visitBinary(const BinaryExpressionNode *node) override;
  bool visitTernary(const TernaryExpressionNode *node) override;
  bool visitAssignment(const AssignmentExpressionNode *node) override;
  bool visitGroup(const GroupExpressionNode *node) override;
  bool visitStmt(const StmtExpressionNode *node) override;
  bool visitStringExpr(const StringExpressionNode *node) override;
  bool visitCast(const CastExpressionNode *node) override;
  bool visitCall(const CallExpressionNode *node) override;
  bool visitIndex(const IndexExpressionNode *node) override;
  bool visitArray(const ArrayExpressionNode *node) override;
  bool visitTuple(const TupleExpressionNode *node) override;
  bool visitStruct(const StructExpressionNode *node) override;
  bool visitMember(const MemberExpressionNode *node) override;
  bool visitMacroCall(const MacroCallExpressionNode *node) override;
  bool visitClosure(const ClosureExpressionNode *node) override;
  bool visitRange(const RangeExpressionNode *node) override;
  bool visitSpread(const SpreadExpressionNode *node) override;

private:
  // Helper methods
  void printNodeStart(const ASTNode *node);
  void printNodeEnd();
  void printNodeStartInline(const ASTNode *node);
  void printLocation(const Location &loc);
  void printType(const Type *type);
  void printFlags(Flags flags);
  void printMetadata(const ASTNode *node);
  void printIndent();
  void increaseIndent();
  void decreaseIndent();
  void printSpace();
  void printNewline();
  bool shouldPrintNode(const ASTNode *node) const;
  bool shouldPrintInline(const ASTNode *node) const;
  bool isCompactMode() const;

  // State management
  void resetState();
  void updateStatistics(const ASTNode *node);

  PrinterConfig config_;
  std::ostream *output_;
  std::ostringstream defaultStream_;
  uint32_t currentDepth_ = 0;
  uint32_t indentLevel_ = 0;
  uint32_t nodesVisited_ = 0;
  uint32_t maxDepthReached_ = 0;
  bool needsIndent_ = true;
  bool isFirstChild_ = true;
  ArenaAllocator *arena_ = nullptr; // Optional arena for temporary allocations
};

// Utility functions
std::string printAST(const ASTNode *root,
                     const PrinterConfig &config = {PrinterFlags::None});
void printASTToFile(const ASTNode *root, const std::string &filename,
                    const PrinterConfig &config = {PrinterFlags::None});

} // namespace cxy::ast
