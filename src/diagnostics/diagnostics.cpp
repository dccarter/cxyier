#include <cxy/diagnostics.hpp>

#include <algorithm>
#include <iostream>
#include <sstream>

namespace cxy {

// ANSI color codes
namespace Colors {
constexpr const char *RESET = "\033[0m";
constexpr const char *RED = "\033[31m";
constexpr const char *YELLOW = "\033[33m";
constexpr const char *BLUE = "\033[34m";
constexpr const char *BRIGHT_RED = "\033[91m";
constexpr const char *DIM = "\033[2m";
} // namespace Colors

// ConsoleDiagnosticSink implementation
ConsoleDiagnosticSink::ConsoleDiagnosticSink(bool colors, SourceManager *srcMgr)
    : useColors(colors), sourceManager(srcMgr) {}

void ConsoleDiagnosticSink::emit(const DiagnosticMessage &msg) {
  std::string severityStr;
  switch (msg.severity) {
  case Severity::Info:
    severityStr = "info";
    break;
  case Severity::Warning:
    severityStr = "warning";
    break;
  case Severity::Error:
    severityStr = "error";
    break;
  case Severity::Fatal:
    severityStr = "fatal";
    break;
  }

  std::cout << getSeverityColor(msg.severity) << severityStr << ": "
            << getResetColor() << msg.message << "\n";

  // Print location header
  std::cout << formatLocationHeader(msg.primaryLocation) << "\n";

  // Print source line with caret if available
  if (sourceManager && sourceManager->hasFile(msg.primaryLocation.filename)) {
    auto sourceLine = getSourceLine(msg.primaryLocation);
    if (!sourceLine.empty()) {
      std::cout << "     |\n";
      std::cout << std::format("{:4} | {}\n", msg.primaryLocation.start.row,
                               sourceLine);
      std::cout << "     | " << createCaretLine(msg.primaryLocation, sourceLine)
                << "\n";
    }
  }

  // Print secondary locations
  for (const auto &loc : msg.secondaryLocations) {
    std::cout << "note: see " << formatLocationHeader(loc) << "\n";

    if (sourceManager && sourceManager->hasFile(loc.filename)) {
      auto sourceLine = getSourceLine(loc);
      if (!sourceLine.empty()) {
        std::cout << "     |\n";
        std::cout << std::format("{:4} | {}\n", loc.start.row, sourceLine);
        std::cout << "     | " << createCaretLine(loc, sourceLine) << "\n";
      }
    }
  }

  // Print notes
  for (const auto &note : msg.notes) {
    std::cout << "note: " << note << "\n";
  }

  // Print suggestion
  if (msg.suggestion) {
    std::cout << "suggestion: " << *msg.suggestion << "\n";
  }

  std::cout << "\n"; // Add blank line after each diagnostic
}

void ConsoleDiagnosticSink::flush() { std::cout << std::flush; }

std::string ConsoleDiagnosticSink::getSeverityColor(Severity severity) const {
  if (!useColors)
    return "";

  switch (severity) {
  case Severity::Info:
    return Colors::BLUE;
  case Severity::Warning:
    return Colors::YELLOW;
  case Severity::Error:
    return Colors::RED;
  case Severity::Fatal:
    return Colors::BRIGHT_RED;
  }
  return "";
}

std::string ConsoleDiagnosticSink::getResetColor() const {
  return useColors ? Colors::RESET : "";
}

std::string
ConsoleDiagnosticSink::formatLocationHeader(const Location &loc) const {
  std::string header = "     in " + loc.filename + ":" +
                       std::to_string(loc.start.row) + ":" +
                       std::to_string(loc.start.column);

  if (!loc.isSinglePosition()) {
    header += " (to " + std::to_string(loc.end.row) + ":" +
              std::to_string(loc.end.column) + ")";
  }

  return (useColors ? Colors::DIM : "") + header +
         (useColors ? Colors::RESET : "");
}

std::string ConsoleDiagnosticSink::getSourceLine(const Location &loc) const {
  if (!sourceManager)
    return "";

  auto line = sourceManager->getLine(loc.filename, loc.start.row);
  return line.value_or("");
}

std::string
ConsoleDiagnosticSink::createCaretLine(const Location &loc,
                                       const std::string &sourceLine) const {
  if (sourceLine.empty())
    return "";

  std::string caret(sourceLine.length(), ' ');

  if (loc.isSinglePosition()) {
    // Single position - just one caret
    if (loc.start.column > 0 && loc.start.column <= sourceLine.length()) {
      caret[loc.start.column - 1] = '^';
    }
  } else if (loc.start.row == loc.end.row) {
    // Single line span
    size_t startCol = std::max(1UL, loc.start.column) - 1;
    size_t endCol =
        std::min(static_cast<size_t>(loc.end.column), sourceLine.length());

    if (startCol < endCol) {
      for (size_t i = startCol; i < endCol; ++i) {
        caret[i] = '^';
      }
    }
  } else {
    // Multi-line span - show from start to end of line
    size_t startCol = std::max(1UL, loc.start.column) - 1;
    for (size_t i = startCol; i < sourceLine.length(); ++i) {
      caret[i] = '^';
    }
  }

  // Apply color if enabled
  std::string coloredCaret = caret;
  if (useColors) {
    // Find first and last non-space characters
    auto first = coloredCaret.find('^');
    auto last = coloredCaret.rfind('^');

    if (first != std::string::npos) {
      std::string color = Colors::RED; // Default to red
      coloredCaret = coloredCaret.substr(0, first) + color +
                     coloredCaret.substr(first, last - first + 1) +
                     Colors::RESET + coloredCaret.substr(last + 1);
    }
  }

  return coloredCaret;
}

// InMemoryDiagnosticSink implementation
void InMemoryDiagnosticSink::emit(const DiagnosticMessage &msg) {
  messages.push_back(msg);
}

void InMemoryDiagnosticSink::flush() {
  // Nothing to flush for in-memory storage
}

size_t InMemoryDiagnosticSink::getErrorCount() const {
  return std::count_if(messages.begin(), messages.end(),
                       [](const DiagnosticMessage &msg) {
                         return msg.severity == Severity::Error;
                       });
}

size_t InMemoryDiagnosticSink::getWarningCount() const {
  return std::count_if(messages.begin(), messages.end(),
                       [](const DiagnosticMessage &msg) {
                         return msg.severity == Severity::Warning;
                       });
}

size_t InMemoryDiagnosticSink::getInfoCount() const {
  return std::count_if(messages.begin(), messages.end(),
                       [](const DiagnosticMessage &msg) {
                         return msg.severity == Severity::Info;
                       });
}

size_t InMemoryDiagnosticSink::getFatalCount() const {
  return std::count_if(messages.begin(), messages.end(),
                       [](const DiagnosticMessage &msg) {
                         return msg.severity == Severity::Fatal;
                       });
}

bool InMemoryDiagnosticSink::hasMessage(
    Severity severity, const std::string &messageSubstring) const {
  return std::any_of(
      messages.begin(), messages.end(),
      [severity, &messageSubstring](const DiagnosticMessage &msg) {
        return msg.severity == severity &&
               msg.message.find(messageSubstring) != std::string::npos;
      });
}

bool InMemoryDiagnosticSink::hasErrorContaining(
    const std::string &messageSubstring) const {
  return hasMessage(Severity::Error, messageSubstring);
}

bool InMemoryDiagnosticSink::hasWarningContaining(
    const std::string &messageSubstring) const {
  return hasMessage(Severity::Warning, messageSubstring);
}

std::vector<DiagnosticMessage>
InMemoryDiagnosticSink::getMessagesBySeverity(Severity severity) const {
  std::vector<DiagnosticMessage> result;
  std::copy_if(messages.begin(), messages.end(), std::back_inserter(result),
               [severity](const DiagnosticMessage &msg) {
                 return msg.severity == severity;
               });
  return result;
}

std::vector<DiagnosticMessage> InMemoryDiagnosticSink::getErrors() const {
  return getMessagesBySeverity(Severity::Error);
}

std::vector<DiagnosticMessage> InMemoryDiagnosticSink::getWarnings() const {
  return getMessagesBySeverity(Severity::Warning);
}

// SourceManager implementation
void SourceManager::registerFile(const std::string &filename,
                                 std::string content) {
  fileContents[filename] = std::move(content);
  buildLineOffsets(filename);
}

std::optional<std::string> SourceManager::getLine(const std::string &filename,
                                                  size_t lineNumber) const {
  auto contentIt = fileContents.find(filename);
  if (contentIt == fileContents.end()) {
    return std::nullopt;
  }

  auto offsetsIt = lineOffsets.find(filename);
  if (offsetsIt == lineOffsets.end()) {
    return std::nullopt;
  }

  const auto &offsets = offsetsIt->second;
  if (lineNumber == 0 || lineNumber > offsets.size()) {
    return std::nullopt;
  }

  const std::string &content = contentIt->second;
  size_t startOffset = offsets[lineNumber - 1];
  size_t endOffset = (lineNumber < offsets.size())
                         ? offsets[lineNumber] - 1
                         : content.length(); // -1 to exclude newline

  if (startOffset >= content.length()) {
    return "";
  }

  return content.substr(startOffset, endOffset - startOffset);
}

std::optional<std::string>
SourceManager::getRange(const Location &location) const {
  auto contentIt = fileContents.find(location.filename);
  if (contentIt == fileContents.end()) {
    return std::nullopt;
  }

  const std::string &content = contentIt->second;
  size_t start = location.start.byteOffset;
  size_t length = location.getLength();

  if (start >= content.length()) {
    return "";
  }

  if (start + length > content.length()) {
    length = content.length() - start;
  }

  return content.substr(start, length);
}

std::string_view SourceManager::getRangeView(const Location &location) const {
  auto contentIt = fileContents.find(location.filename);
  if (contentIt == fileContents.end()) {
    return {}; // Return empty string_view
  }

  const std::string &content = contentIt->second;
  size_t start = location.start.byteOffset;
  size_t length = location.getLength();

  if (start >= content.length()) {
    return {};
  }

  if (start + length > content.length()) {
    length = content.length() - start;
  }

  return std::string_view(content.data() + start, length);
}

Position SourceManager::createPosition(const std::string &filename,
                                       size_t byteOffset) {
  auto [row, column] = getLineAndColumn(filename, byteOffset);
  return Position(row, column, byteOffset);
}

bool SourceManager::hasFile(const std::string &filename) const {
  return fileContents.find(filename) != fileContents.end();
}

const std::string *
SourceManager::getFileContent(const std::string &filename) const {
  auto it = fileContents.find(filename);
  return (it != fileContents.end()) ? &it->second : nullptr;
}

void SourceManager::buildLineOffsets(const std::string &filename) {
  auto contentIt = fileContents.find(filename);
  if (contentIt == fileContents.end()) {
    return;
  }

  const std::string &content = contentIt->second;
  std::vector<size_t> &offsets = lineOffsets[filename];
  offsets.clear();
  offsets.push_back(0); // First line starts at offset 0

  for (size_t i = 0; i < content.length(); ++i) {
    if (content[i] == '\n') {
      offsets.push_back(i + 1); // Next line starts after the newline
    }
  }
}

std::pair<size_t, size_t>
SourceManager::getLineAndColumn(const std::string &filename,
                                size_t byteOffset) const {
  auto offsetsIt = lineOffsets.find(filename);
  if (offsetsIt == lineOffsets.end()) {
    return {1, 1}; // Default position
  }

  const auto &offsets = offsetsIt->second;

  // Find the line containing this byte offset
  auto it = std::upper_bound(offsets.begin(), offsets.end(), byteOffset);
  if (it == offsets.begin()) {
    return {1, byteOffset + 1}; // First line
  }

  --it; // Go back to the line start
  size_t lineNumber = std::distance(offsets.begin(), it) + 1;
  size_t columnNumber = byteOffset - *it + 1;

  return {lineNumber, columnNumber};
}

// DiagnosticLogger implementation
DiagnosticLogger::DiagnosticLogger()
    : errorCount(0), warningCount(0), fatalCount(0) {
  // Add default console sink
  addSink(std::make_unique<ConsoleDiagnosticSink>());
}

void DiagnosticLogger::addSink(std::unique_ptr<DiagnosticSink> sink) {
  sinks.push_back(std::move(sink));
}

void DiagnosticLogger::removeAllSinks() { sinks.clear(); }

void DiagnosticLogger::error(const std::string &message,
                             const Location &location) {
  emit(DiagnosticMessage(Severity::Error, message, location));
}

void DiagnosticLogger::warning(const std::string &message,
                               const Location &location) {
  emit(DiagnosticMessage(Severity::Warning, message, location));
}

void DiagnosticLogger::info(const std::string &message,
                            const Location &location) {
  emit(DiagnosticMessage(Severity::Info, message, location));
}

void DiagnosticLogger::fatal(const std::string &message,
                             const Location &location) {
  emit(DiagnosticMessage(Severity::Fatal, message, location));
}

void DiagnosticLogger::emit(DiagnosticMessage msg) {
  // Update counters
  switch (msg.severity) {
  case Severity::Error:
    ++errorCount;
    break;
  case Severity::Warning:
    ++warningCount;
    break;
  case Severity::Fatal:
    ++fatalCount;
    break;
  case Severity::Info:
    break; // Don't count info messages
  }

  // Emit to all sinks
  for (auto &sink : sinks) {
    sink->emit(msg);
  }
}

void DiagnosticLogger::flush() {
  for (auto &sink : sinks) {
    sink->flush();
  }
}

void DiagnosticLogger::resetCounters() {
  errorCount = 0;
  warningCount = 0;
  fatalCount = 0;
}

} // namespace cxy
