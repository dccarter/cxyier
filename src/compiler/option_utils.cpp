#include <cxy/compiler/option_parser.hpp>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace cxy::compiler {

std::string commandToString(Command cmd) {
    switch (cmd) {
        case Command::Dev:     return "dev";
        case Command::Build:   return "build";
        case Command::Test:    return "test";
        case Command::Run:     return "run";
        case Command::Check:   return "check";
        case Command::Help:    return "help";
        case Command::Version: return "version";
    }
    return "unknown";
}

std::optional<Command> stringToCommand(std::string_view str) {
    // Convert to lowercase for case-insensitive comparison
    std::string lowerStr;
    lowerStr.reserve(str.size());
    std::transform(str.begin(), str.end(), std::back_inserter(lowerStr),
                   [](char c) { return std::tolower(c); });
    
    if (lowerStr == "dev") return Command::Dev;
    if (lowerStr == "build") return Command::Build;
    if (lowerStr == "test") return Command::Test;
    if (lowerStr == "run") return Command::Run;
    if (lowerStr == "check") return Command::Check;
    if (lowerStr == "help") return Command::Help;
    if (lowerStr == "version") return Command::Version;
    
    return std::nullopt;
}

std::vector<std::filesystem::path> getDefaultConfigPaths() {
    std::vector<std::filesystem::path> paths;
    
    // Current directory
    paths.emplace_back("cxy.toml");
    paths.emplace_back(".cxy/config.toml");
    
    // Home directory
    if (const char* home = std::getenv("HOME")) {
        std::filesystem::path homeDir(home);
        paths.emplace_back(homeDir / ".config" / "cxy" / "config.toml");
        paths.emplace_back(homeDir / ".cxy" / "config.toml");
    }
    
    // Windows-specific paths
    if (const char* appdata = std::getenv("APPDATA")) {
        std::filesystem::path appdataDir(appdata);
        paths.emplace_back(appdataDir / "cxy" / "config.toml");
    }
    
    // Unix system-wide paths
    paths.emplace_back("/etc/cxy/config.toml");
    paths.emplace_back("/usr/local/etc/cxy/config.toml");
    
    return paths;
}

std::unique_ptr<OptionParser> createDefaultOptionParser() {
    // Create a diagnostic logger with console sink
    static auto diagnostics = std::make_unique<DiagnosticLogger>();
    if (!diagnostics) {
        diagnostics = std::make_unique<DiagnosticLogger>();
        // Add default console sink
        auto consoleSink = std::make_unique<ConsoleDiagnosticSink>();
        diagnostics->addSink(std::move(consoleSink));
    }
    
    return std::make_unique<OptionParser>(*diagnostics);
}

} // namespace cxy::compiler