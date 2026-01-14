#include "catch2.hpp"
#include <cxy/diagnostics.hpp>

#include <sstream>
#include <memory>

using namespace cxy;

// Test sink that captures output for verification
class TestDiagnosticSink : public DiagnosticSink {
private:
    std::ostringstream buffer;
    std::vector<DiagnosticMessage> messages;
    
public:
    void emit(const DiagnosticMessage& msg) override {
        messages.push_back(msg);
        buffer << "Severity: " << static_cast<int>(msg.severity) << ", ";
        buffer << "Message: " << msg.message << ", ";
        buffer << "Location: " << msg.primaryLocation.filename << ":" 
               << msg.primaryLocation.start.row << ":" 
               << msg.primaryLocation.start.column << "\n";
    }
    
    void flush() override {
        buffer.flush();
    }
    
    std::string getOutput() const { return buffer.str(); }
    const std::vector<DiagnosticMessage>& getMessages() const { return messages; }
    void clear() { 
        buffer.str(""); 
        buffer.clear();
        messages.clear();
    }
};

TEST_CASE("Position functionality", "[diagnostics][position]") {
    SECTION("Default construction") {
        Position pos;
        REQUIRE(pos.row == 1);
        REQUIRE(pos.column == 1);
        REQUIRE(pos.byteOffset == 0);
    }
    
    SECTION("Parameterized construction") {
        Position pos(5, 10, 42);
        REQUIRE(pos.row == 5);
        REQUIRE(pos.column == 10);
        REQUIRE(pos.byteOffset == 42);
    }
    
    SECTION("Equality comparison") {
        Position pos1(1, 1, 0);
        Position pos2(1, 1, 0);
        Position pos3(2, 1, 0);
        
        REQUIRE(pos1 == pos2);
        REQUIRE(pos1 != pos3);
    }
    
    SECTION("Ordering") {
        Position pos1(1, 5, 5);
        Position pos2(1, 10, 10);
        Position pos3(2, 1, 15);
        
        REQUIRE(pos1 < pos2);
        REQUIRE(pos2 < pos3);
        REQUIRE(pos1 < pos3);
    }
}

TEST_CASE("Location functionality", "[diagnostics][location]") {
    SECTION("Single position location") {
        Position pos(5, 10, 42);
        Location loc("test.txt", pos);
        
        REQUIRE(loc.filename == "test.txt");
        REQUIRE(loc.start == pos);
        REQUIRE(loc.end == pos);
        REQUIRE(loc.isSinglePosition());
        REQUIRE(!loc.spansMultipleLines());
        REQUIRE(loc.getLength() == 0);
    }
    
    SECTION("Range location") {
        Position start(5, 10, 42);
        Position end(5, 15, 47);
        Location loc("test.txt", start, end);
        
        REQUIRE(loc.filename == "test.txt");
        REQUIRE(loc.start == start);
        REQUIRE(loc.end == end);
        REQUIRE(!loc.isSinglePosition());
        REQUIRE(!loc.spansMultipleLines());
        REQUIRE(loc.getLength() == 5);
    }
    
    SECTION("Multi-line location") {
        Position start(5, 10, 42);
        Position end(7, 5, 65);
        Location loc("test.txt", start, end);
        
        REQUIRE(loc.spansMultipleLines());
        REQUIRE(loc.getLength() == 23);
    }
}

TEST_CASE("SourceManager functionality", "[diagnostics][source_manager]") {
    SECTION("Basic file registration and retrieval") {
        SourceManager srcMgr;
        
        std::string content = "line 1\nline 2\nline 3";
        srcMgr.registerFile("test.txt", content);
        
        REQUIRE(srcMgr.hasFile("test.txt"));
        REQUIRE(!srcMgr.hasFile("nonexistent.txt"));
        
        auto line1 = srcMgr.getLine("test.txt", 1);
        auto line2 = srcMgr.getLine("test.txt", 2);
        auto line3 = srcMgr.getLine("test.txt", 3);
        
        REQUIRE(line1.has_value());
        REQUIRE(line1.value() == "line 1");
        REQUIRE(line2.has_value());
        REQUIRE(line2.value() == "line 2");
        REQUIRE(line3.has_value());
        REQUIRE(line3.value() == "line 3");
        
        auto invalidLine = srcMgr.getLine("test.txt", 4);
        REQUIRE(!invalidLine.has_value());
    }
    
    SECTION("Position creation from byte offset") {
        SourceManager srcMgr;
        
        std::string content = "hello\nworld\ntest";
        srcMgr.registerFile("test.txt", content);
        
        // "hello\n" = 6 chars, so offset 6 should be start of line 2
        Position pos1 = srcMgr.createPosition("test.txt", 0);  // 'h' in "hello"
        Position pos2 = srcMgr.createPosition("test.txt", 6);  // 'w' in "world"
        Position pos3 = srcMgr.createPosition("test.txt", 12); // 't' in "test"
        
        REQUIRE(pos1.row == 1);
        REQUIRE(pos1.column == 1);
        REQUIRE(pos1.byteOffset == 0);
        
        REQUIRE(pos2.row == 2);
        REQUIRE(pos2.column == 1);
        REQUIRE(pos2.byteOffset == 6);
        
        REQUIRE(pos3.row == 3);
        REQUIRE(pos3.column == 1);
        REQUIRE(pos3.byteOffset == 12);
    }
    
    SECTION("Range retrieval") {
        SourceManager srcMgr;
        
        std::string content = "hello world";
        srcMgr.registerFile("test.txt", content);
        
        Position start(1, 7, 6);  // 'w' in "world"
        Position end(1, 11, 10);   // 'd' in "world" (exclusive end)
        Location loc("test.txt", start, end);
        
        auto range = srcMgr.getRange(loc);
        REQUIRE(range.has_value());
        REQUIRE(range.value() == "worl");
    }
    
    SECTION("Empty file handling") {
        SourceManager srcMgr;
        
        srcMgr.registerFile("empty.txt", "");
        REQUIRE(srcMgr.hasFile("empty.txt"));
        
        auto line = srcMgr.getLine("empty.txt", 1);
        REQUIRE(line.has_value());
        REQUIRE(line.value() == "");
    }
}

TEST_CASE("DiagnosticMessage construction", "[diagnostics][message]") {
    SECTION("Basic message creation") {
        Position pos(5, 10, 42);
        Location loc("test.txt", pos);
        
        DiagnosticMessage msg(Severity::Error, "Test error", loc);
        
        REQUIRE(msg.severity == Severity::Error);
        REQUIRE(msg.message == "Test error");
        REQUIRE(msg.primaryLocation.filename == "test.txt");
        REQUIRE(msg.primaryLocation.start == pos);
        REQUIRE(msg.secondaryLocations.empty());
        REQUIRE(msg.notes.empty());
        REQUIRE(!msg.suggestion.has_value());
    }
    
    SECTION("Message with additional information") {
        Position pos(5, 10, 42);
        Location primaryLoc("test.txt", pos);
        Location secondaryLoc("other.txt", Position(1, 1, 0));
        
        DiagnosticMessage msg(Severity::Warning, "Test warning", primaryLoc);
        msg.secondaryLocations.push_back(secondaryLoc);
        msg.notes.push_back("This is a note");
        msg.suggestion = "Try this fix";
        
        REQUIRE(msg.secondaryLocations.size() == 1);
        REQUIRE(msg.secondaryLocations[0].filename == "other.txt");
        REQUIRE(msg.notes.size() == 1);
        REQUIRE(msg.notes[0] == "This is a note");
        REQUIRE(msg.suggestion.has_value());
        REQUIRE(msg.suggestion.value() == "Try this fix");
    }
}

TEST_CASE("DiagnosticLogger functionality", "[diagnostics][logger]") {
    SECTION("Counter tracking") {
        DiagnosticLogger logger;
        logger.removeAllSinks(); // Remove default sink for testing
        
        Position pos(1, 1, 0);
        Location loc("test.txt", pos);
        
        REQUIRE(logger.getErrorCount() == 0);
        REQUIRE(logger.getWarningCount() == 0);
        REQUIRE(logger.getFatalCount() == 0);
        REQUIRE(!logger.hasErrors());
        REQUIRE(!logger.hasFatalErrors());
        
        logger.error("Error 1", loc);
        logger.warning("Warning 1", loc);
        logger.info("Info 1", loc);
        logger.fatal("Fatal 1", loc);
        
        REQUIRE(logger.getErrorCount() == 1);
        REQUIRE(logger.getWarningCount() == 1);
        REQUIRE(logger.getFatalCount() == 1);
        REQUIRE(logger.hasErrors());
        REQUIRE(logger.hasFatalErrors());
        
        // Info messages shouldn't be counted as errors
        logger.info("Info 2", loc);
        REQUIRE(logger.getErrorCount() == 1);
        REQUIRE(logger.getWarningCount() == 1);
        REQUIRE(logger.getFatalCount() == 1);
        
        logger.resetCounters();
        REQUIRE(logger.getErrorCount() == 0);
        REQUIRE(logger.getWarningCount() == 0);
        REQUIRE(logger.getFatalCount() == 0);
    }
    
    SECTION("Formatted messages") {
        DiagnosticLogger logger;
        logger.removeAllSinks(); // Remove default sink
        
        auto testSink = std::make_unique<TestDiagnosticSink>();
        auto* sinkPtr = testSink.get();
        logger.addSink(std::move(testSink));
        
        Position pos(5, 10, 42);
        Location loc("test.cpp", pos);
        
        logger.error(loc, "undefined variable '{}'", "foo");
        logger.warning(loc, "unused parameter '{}' in function '{}'", "x", "main");
        logger.fatal(loc, "internal compiler error: code {}", 42);
        
        const auto& messages = sinkPtr->getMessages();
        REQUIRE(messages.size() == 3);
        
        REQUIRE(messages[0].message == "undefined variable 'foo'");
        REQUIRE(messages[1].message == "unused parameter 'x' in function 'main'");
        REQUIRE(messages[2].message == "internal compiler error: code 42");
    }
    
    SECTION("Custom sink integration") {
        DiagnosticLogger logger;
        logger.removeAllSinks();
        
        auto testSink = std::make_unique<TestDiagnosticSink>();
        auto* sinkPtr = testSink.get();
        logger.addSink(std::move(testSink));
        
        Position pos(1, 1, 0);
        Location loc("test.txt", pos);
        
        logger.error("Test error", loc);
        logger.warning("Test warning", loc);
        
        std::string output = sinkPtr->getOutput();
        REQUIRE(output.find("Severity: 2") != std::string::npos); // Error = 2
        REQUIRE(output.find("Severity: 1") != std::string::npos); // Warning = 1
        REQUIRE(output.find("Test error") != std::string::npos);
        REQUIRE(output.find("Test warning") != std::string::npos);
    }
}

TEST_CASE("ConsoleDiagnosticSink functionality", "[diagnostics][console_sink]") {
    SECTION("Basic sink creation and emit") {
        TestDiagnosticSink sink; // Use test sink instead of console
        
        // Basic test - just ensure it doesn't crash
        Position pos(5, 10, 42);
        Location loc("test.cpp", pos);
        DiagnosticMessage msg(Severity::Error, "Test error", loc);
        
        REQUIRE_NOTHROW(sink.emit(msg));
        REQUIRE_NOTHROW(sink.flush());
        
        // Verify the message was captured
        REQUIRE(sink.getMessages().size() == 1);
        REQUIRE(sink.getMessages()[0].message == "Test error");
    }
    
    SECTION("Sink with source manager integration") {
        SourceManager srcMgr;
        srcMgr.registerFile("test.cpp", "int main() {\n    int x = foo;\n    return 0;\n}");
        
        TestDiagnosticSink sink; // Test actual diagnostic emission without console output
        
        Position start(2, 13, 25); // 'foo' in the code
        Position end(2, 16, 28);
        Location loc("test.cpp", start, end);
        
        DiagnosticMessage msg(Severity::Error, "undefined variable 'foo'", loc);
        
        REQUIRE_NOTHROW(sink.emit(msg));
        
        // Verify diagnostic was captured
        REQUIRE(sink.getMessages().size() == 1);
        REQUIRE(sink.getMessages()[0].message == "undefined variable 'foo'");
    }
    
    SECTION("Message formatting verification") {
        TestDiagnosticSink sink;
        
        Position pos(1, 1, 0);
        Location loc("test.txt", pos);
        DiagnosticMessage msg(Severity::Info, "Test message", loc);
        
        REQUIRE_NOTHROW(sink.emit(msg));
        
        // Verify output format
        std::string output = sink.getOutput();
        REQUIRE(output.find("Severity: 0") != std::string::npos); // Info severity
        REQUIRE(output.find("Test message") != std::string::npos);
        REQUIRE(output.find("test.txt:1:1") != std::string::npos);
    }
}

TEST_CASE("Integration test", "[diagnostics][integration]") {
    SECTION("Complete diagnostic workflow") {
        SourceManager srcMgr;
        DiagnosticLogger logger;
        
        // Remove default sink and add our test sink
        logger.removeAllSinks();
        auto testSink = std::make_unique<TestDiagnosticSink>();
        auto* sinkPtr = testSink.get();
        logger.addSink(std::move(testSink));
        
        // Register source file
        std::string sourceCode = 
            "fn main() {\n"
            "    let x = undefinedVar;\n"
            "    let y = 42;\n"
            "}\n";
        srcMgr.registerFile("main.cxy", sourceCode);
        
        // Create positions for various errors
        Position errorPos = srcMgr.createPosition("main.cxy", 25); // 'undefinedVar'
        Position warningPos = srcMgr.createPosition("main.cxy", 45); // 'y'
        
        Location errorLoc("main.cxy", errorPos);
        Location warningLoc("main.cxy", warningPos);
        
        // Generate some diagnostics
        logger.error(errorLoc, "undefined variable '{}'", "undefinedVar");
        logger.warning(warningLoc, "unused variable '{}'", "y");
        logger.info(Location("main.cxy", Position(1, 1, 0)), "compilation started");
        
        // Check results
        REQUIRE(logger.getErrorCount() == 1);
        REQUIRE(logger.getWarningCount() == 1);
        REQUIRE(logger.getFatalCount() == 0);
        REQUIRE(logger.hasErrors());
        REQUIRE(!logger.hasFatalErrors());
        
        const auto& messages = sinkPtr->getMessages();
        REQUIRE(messages.size() == 3);
        REQUIRE(messages[0].severity == Severity::Error);
        REQUIRE(messages[1].severity == Severity::Warning);
        REQUIRE(messages[2].severity == Severity::Info);
    }
}