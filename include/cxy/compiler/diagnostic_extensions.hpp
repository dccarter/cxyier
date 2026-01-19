#pragma once

#include <cxy/diagnostics.hpp>
#include <format>
#include <string>

namespace cxy::compiler {

/// Extended diagnostic functions for option parsing that don't require source locations
class DiagnosticExtensions {
private:
    DiagnosticLogger& logger_;
    
public:
    explicit DiagnosticExtensions(DiagnosticLogger& logger) : logger_(logger) {}
    
    /// Emit error without source location (for command-line parsing errors)
    template<typename... Args>
    void error(std::format_string<Args...> format, Args&&... args) {
        std::string message = std::format(format, std::forward<Args>(args)...);
        DiagnosticMessage msg(Severity::Error, message, Location{});
        logger_.emit(std::move(msg));
    }
    
    /// Emit error with string message
    void error(const std::string& message) {
        DiagnosticMessage msg(Severity::Error, message, Location{});
        logger_.emit(std::move(msg));
    }
    
    /// Emit warning without source location
    template<typename... Args>
    void warning(std::format_string<Args...> format, Args&&... args) {
        std::string message = std::format(format, std::forward<Args>(args)...);
        DiagnosticMessage msg(Severity::Warning, message, Location{});
        logger_.emit(std::move(msg));
    }
    
    /// Emit warning with string message
    void warning(const std::string& message) {
        DiagnosticMessage msg(Severity::Warning, message, Location{});
        logger_.emit(std::move(msg));
    }
    
    /// Emit info without source location
    template<typename... Args>
    void info(std::format_string<Args...> format, Args&&... args) {
        std::string message = std::format(format, std::forward<Args>(args)...);
        DiagnosticMessage msg(Severity::Info, message, Location{});
        logger_.emit(std::move(msg));
    }
    
    /// Emit info with string message
    void info(const std::string& message) {
        DiagnosticMessage msg(Severity::Info, message, Location{});
        logger_.emit(std::move(msg));
    }
    
    /// Get reference to underlying logger
    DiagnosticLogger& getLogger() { return logger_; }
    const DiagnosticLogger& getLogger() const { return logger_; }
    
    /// Convenience accessors for statistics
    bool hasErrors() const { return logger_.hasErrors(); }
    size_t getErrorCount() const { return logger_.getErrorCount(); }
    size_t getWarningCount() const { return logger_.getWarningCount(); }
    
    /// Flush all sinks
    void flush() { logger_.flush(); }
};

/// Helper function to create diagnostic extensions
inline DiagnosticExtensions makeDiagnosticExtensions(DiagnosticLogger& logger) {
    return DiagnosticExtensions(logger);
}

} // namespace cxy::compiler