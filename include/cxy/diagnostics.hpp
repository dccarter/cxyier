#pragma once

#include <cstddef>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace cxy {

// Forward declarations
class SourceManager;
class DiagnosticSink;

struct Position {
  size_t row;        // 1-based line number
  size_t column;     // 1-based column number
  size_t byteOffset; // 0-based byte offset from start of file

  Position() : row(1), column(1), byteOffset(0) {}
  Position(size_t r, size_t c, size_t offset)
      : row(r), column(c), byteOffset(offset) {}

  bool operator==(const Position &other) const noexcept {
    return row == other.row && column == other.column &&
           byteOffset == other.byteOffset;
  }

  bool operator!=(const Position &other) const noexcept {
    return !(*this == other);
  }

  auto operator<=>(const Position &other) const noexcept {
    if (auto cmp = row <=> other.row; cmp != 0)
      return cmp;
    if (auto cmp = column <=> other.column; cmp != 0)
      return cmp;
    return byteOffset <=> other.byteOffset;
  }
};

struct Location {
  std::string filename;
  Position start;
  Position end;

  Location() = default;
  Location(std::string file, Position s, Position e)
      : filename(std::move(file)), start(s), end(e) {}

  // Single position constructor
  Location(std::string file, Position pos)
      : filename(std::move(file)), start(pos), end(pos) {}

  bool operator==(const Location &other) const noexcept {
    return filename == other.filename && start == other.start &&
           end == other.end;
  }

  bool operator!=(const Location &other) const noexcept {
    return !(*this == other);
  }

  // Helper methods
  [[nodiscard]] bool isSinglePosition() const noexcept { return start == end; }

  [[nodiscard]] bool spansMultipleLines() const noexcept {
    return start.row != end.row;
  }

  [[nodiscard]] size_t getLength() const noexcept {
    return end.byteOffset - start.byteOffset;
  }
};

enum class Severity { Info, Warning, Error, Fatal };

struct DiagnosticMessage {
  Severity severity;
  std::string message;
  Location primaryLocation;
  std::vector<Location> secondaryLocations; // For "see also" references
  std::vector<std::string> notes;           // Additional context
  std::optional<std::string> suggestion;    // Fix suggestion

  DiagnosticMessage(Severity sev, std::string msg, Location loc)
      : severity(sev), message(std::move(msg)),
        primaryLocation(std::move(loc)) {}
};

class DiagnosticSink {
public:
  virtual ~DiagnosticSink() = default;
  virtual void emit(const DiagnosticMessage &msg) = 0;
  virtual void flush() = 0;
};

class ConsoleDiagnosticSink : public DiagnosticSink {
private:
  bool useColors;
  SourceManager *sourceManager; // For retrieving source lines

public:
  explicit ConsoleDiagnosticSink(bool colors = true,
                                 SourceManager *srcMgr = nullptr);

  void emit(const DiagnosticMessage &msg) override;
  void flush() override;

  void setSourceManager(SourceManager *srcMgr) { sourceManager = srcMgr; }
  void setUseColors(bool colors) { useColors = colors; }

private:
  std::string getSeverityColor(Severity severity) const;
  std::string getResetColor() const;
  std::string formatLocationHeader(const Location &loc) const;
  std::string getSourceLine(const Location &loc) const;
  std::string createCaretLine(const Location &loc,
                              const std::string &sourceLine) const;
};

class InMemoryDiagnosticSink : public DiagnosticSink {
private:
  std::vector<DiagnosticMessage> messages;

public:
  void emit(const DiagnosticMessage &msg) override;
  void flush() override;

  // Test helper methods
  [[nodiscard]] const std::vector<DiagnosticMessage> &getMessages() const {
    return messages;
  }
  [[nodiscard]] size_t getMessageCount() const { return messages.size(); }
  [[nodiscard]] size_t getErrorCount() const;
  [[nodiscard]] size_t getWarningCount() const;
  [[nodiscard]] size_t getInfoCount() const;
  [[nodiscard]] size_t getFatalCount() const;

  void clear() { messages.clear(); }

  // Check for specific messages
  [[nodiscard]] bool hasMessage(Severity severity,
                                const std::string &messageSubstring) const;
  [[nodiscard]] bool
  hasErrorContaining(const std::string &messageSubstring) const;
  [[nodiscard]] bool
  hasWarningContaining(const std::string &messageSubstring) const;

  // Get messages by severity
  [[nodiscard]] std::vector<DiagnosticMessage>
  getMessagesBySeverity(Severity severity) const;
  [[nodiscard]] std::vector<DiagnosticMessage> getErrors() const;
  [[nodiscard]] std::vector<DiagnosticMessage> getWarnings() const;
};

class SourceManager {
private:
  std::unordered_map<std::string, std::string> fileContents;
  std::unordered_map<std::string, std::vector<size_t>>
      lineOffsets; // Cache line start offsets

public:
  void registerFile(const std::string &filename, std::string content);
  [[nodiscard]] std::optional<std::string> getLine(const std::string &filename,
                                                   size_t lineNumber) const;
  [[nodiscard]] std::optional<std::string>
  getRange(const Location &location) const;
  [[nodiscard]] std::string_view getRangeView(const Location &location) const;

  // Helper for creating positions during lexing/parsing
  [[nodiscard]] Position createPosition(const std::string &filename,
                                        size_t byteOffset);

  // Check if file is registered
  [[nodiscard]] bool hasFile(const std::string &filename) const;

  // Get file content
  [[nodiscard]] const std::string *
  getFileContent(const std::string &filename) const;

private:
  void buildLineOffsets(const std::string &filename);
  [[nodiscard]] std::pair<size_t, size_t>
  getLineAndColumn(const std::string &filename, size_t byteOffset) const;
};

class DiagnosticLogger {
private:
  std::vector<std::unique_ptr<DiagnosticSink>> sinks;
  size_t errorCount;
  size_t warningCount;
  size_t fatalCount;

public:
  DiagnosticLogger();
  ~DiagnosticLogger() = default;

  // Non-copyable but movable
  DiagnosticLogger(const DiagnosticLogger &) = delete;
  DiagnosticLogger &operator=(const DiagnosticLogger &) = delete;
  DiagnosticLogger(DiagnosticLogger &&) = default;
  DiagnosticLogger &operator=(DiagnosticLogger &&) = default;

  void addSink(std::unique_ptr<DiagnosticSink> sink);
  void removeAllSinks();

  // Simple string message functions
  void error(const std::string &message, const Location &location);
  void warning(const std::string &message, const Location &location);
  void info(const std::string &message, const Location &location);
  void fatal(const std::string &message, const Location &location);

  // Formatted message functions using std::format
  template <typename... Args>
  void error(const Location &location, std::format_string<Args...> format,
             Args &&...args) {
    std::string message = std::format(format, std::forward<Args>(args)...);
    error(message, location);
  }

  template <typename... Args>
  void warning(const Location &location, std::format_string<Args...> format,
               Args &&...args) {
    std::string message = std::format(format, std::forward<Args>(args)...);
    warning(message, location);
  }

  template <typename... Args>
  void info(const Location &location, std::format_string<Args...> format,
            Args &&...args) {
    std::string message = std::format(format, std::forward<Args>(args)...);
    info(message, location);
  }

  template <typename... Args>
  void fatal(const Location &location, std::format_string<Args...> format,
             Args &&...args) {
    std::string message = std::format(format, std::forward<Args>(args)...);
    fatal(message, location);
  }

  // Advanced API
  void emit(DiagnosticMessage msg);

  // Statistics
  [[nodiscard]] size_t getErrorCount() const noexcept { return errorCount; }
  [[nodiscard]] size_t getWarningCount() const noexcept { return warningCount; }
  [[nodiscard]] size_t getFatalCount() const noexcept { return fatalCount; }
  [[nodiscard]] bool hasErrors() const noexcept { return errorCount > 0; }
  [[nodiscard]] bool hasFatalErrors() const noexcept { return fatalCount > 0; }

  void flush();

  // Reset counters
  void resetCounters();
};

} // namespace cxy

// Formatter specialization for Location
template <> struct std::formatter<cxy::Location> {
  constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  auto format(const cxy::Location &loc, format_context &ctx) const {
    return std::format_to(ctx.out(), "{}:{}:{}-{}:{}", loc.filename,
                          loc.start.row, loc.start.column, loc.end.row,
                          loc.end.column);
  }
};
