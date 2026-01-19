#pragma once

#include "options.hpp"
#include <cxy/diagnostics.hpp>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <unordered_map>

namespace cxy::compiler {

/// Result of parsing command line arguments
enum class ParseResult : std::uint8_t {
    Success,        ///< Parsing completed successfully
    Error,          ///< Parse error occurred
    HelpRequested,  ///< Help was requested
    VersionRequested ///< Version was requested
};

/// Option definition for help generation and validation
struct OptionDef {
    std::string longName;           ///< Long option name (without --)
    std::string shortName;          ///< Short option name (without -)
    std::string description;        ///< Help description
    std::string argumentName;       ///< Argument name for help (empty if flag)
    OptionCategory category;        ///< Category for organizing help
    std::vector<Command> validCommands; ///< Commands this option is valid for (empty = all)
    bool requiresValue = false;     ///< Whether option requires a value
    bool isFlag = false;           ///< Whether option is a boolean flag
    std::string defaultValue;       ///< Default value description for help
};

/// Command line option parser for the Cxy compiler
class OptionParser {
public:
    /// Constructor
    /// @param diagnostics Diagnostic logger for error reporting
    explicit OptionParser(DiagnosticLogger& diagnostics);
    
    /// Destructor
    ~OptionParser() = default;
    
    // Non-copyable, movable
    OptionParser(const OptionParser&) = delete;
    OptionParser& operator=(const OptionParser&) = delete;
    OptionParser(OptionParser&&) = default;
    OptionParser& operator=(OptionParser&&) = delete;
    
    /// Parse command line arguments
    /// @param argc Argument count
    /// @param argv Argument vector (will be modified to remove parsed args)
    /// @param options Output options structure
    /// @return Parse result indicating success, error, or special request
    [[nodiscard]] ParseResult parseCommandLine(int& argc, char** argv, 
                                              CompilerOptions& options);
    
    /// Parse configuration file
    /// @param configPath Path to configuration file
    /// @param options Output options structure to merge into
    /// @return true if parsing succeeded, false otherwise
    [[nodiscard]] bool parseConfigFile(const std::filesystem::path& configPath,
                                      CompilerOptions& options);
    
    /// Validate parsed options for consistency and completeness
    /// @param options Options to validate
    /// @return true if options are valid, false otherwise
    [[nodiscard]] bool validateOptions(const CompilerOptions& options);
    
    /// Generate help text for the specified command
    /// @param programName Name of the program
    /// @param cmd Command to generate help for (Command::Help for general help)
    /// @return Formatted help string
    [[nodiscard]] std::string generateHelp(const std::string& programName,
                                          Command cmd = Command::Help) const;
    
    /// Generate version information
    /// @return Version string
    [[nodiscard]] std::string generateVersion() const;
    
    /// Find configuration file in standard locations
    /// @return Path to configuration file if found, nullopt otherwise
    [[nodiscard]] std::optional<std::filesystem::path> findConfigFile() const;

private:
    DiagnosticLogger& diagnostics_;
    std::unordered_map<std::string, OptionDef> optionDefs_;
    
    /// Initialize option definitions
    void initializeOptionDefinitions();
    
    /// Parse a single command line argument
    /// @param arg Current argument
    /// @param args All arguments
    /// @param index Current index (will be modified)
    /// @param options Options to update
    /// @return true if parsing succeeded, false otherwise
    [[nodiscard]] bool parseArgument(std::string_view arg,
                                   const std::vector<std::string>& args,
                                   std::size_t& index,
                                   CompilerOptions& options);
    
    /// Parse short option (single dash)
    /// @param option Option string without the dash
    /// @param args All arguments
    /// @param index Current index (will be modified)
    /// @param options Options to update
    /// @return true if parsing succeeded, false otherwise
    [[nodiscard]] bool parseShortOption(std::string_view option,
                                       const std::vector<std::string>& args,
                                       std::size_t& index,
                                       CompilerOptions& options);
    
    /// Parse long option (double dash)
    /// @param option Option string without the dashes
    /// @param args All arguments
    /// @param index Current index (will be modified)
    /// @param options Options to update
    /// @return true if parsing succeeded, false otherwise
    [[nodiscard]] bool parseLongOption(std::string_view option,
                                      const std::vector<std::string>& args,
                                      std::size_t& index,
                                      CompilerOptions& options);
    
    /// Parse command from first argument
    /// @param cmdString Command string
    /// @param options Options to update
    /// @return true if valid command found, false otherwise
    [[nodiscard]] bool parseCommand(std::string_view cmdString, 
                                  CompilerOptions& options);
    
    /// Apply a parsed option to the options structure
    /// @param optionName Name of the option
    /// @param value Option value (empty for flags)
    /// @param options Options to update
    /// @return true if option was applied successfully, false otherwise
    [[nodiscard]] bool applyOption(const std::string& optionName,
                                 const std::string& value,
                                 CompilerOptions& options);
    
    /// Get next argument value, handling missing values
    /// @param args All arguments
    /// @param index Current index (will be modified)
    /// @param optionName Name of option requesting value
    /// @return Option value, or empty if missing/error
    [[nodiscard]] std::string getOptionValue(const std::vector<std::string>& args,
                                            std::size_t& index,
                                            const std::string& optionName);
    
    /// Report unknown option with suggestions
    /// @param option Unknown option name
    /// @param location Source location for error reporting
    void reportUnknownOption(std::string_view option, 
                           const Location& location = Location{});
    
    /// Suggest similar option names
    /// @param option Option to find suggestions for
    /// @return Vector of similar option names
    [[nodiscard]] std::vector<std::string> suggestSimilarOptions(std::string_view option) const;
    
    /// Check if option is valid for the current command
    /// @param optionName Option to check
    /// @param cmd Current command
    /// @return true if option is valid for command, false otherwise
    [[nodiscard]] bool isOptionValidForCommand(const std::string& optionName,
                                             Command cmd) const;
    
    /// Parse TOML configuration file
    /// @param configPath Path to TOML file
    /// @param options Options to update
    /// @return true if parsing succeeded, false otherwise
    [[nodiscard]] bool parseTomlConfig(const std::filesystem::path& configPath,
                                      CompilerOptions& options);
    
    /// Apply configuration option from file
    /// @param section TOML section name
    /// @param key Option key
    /// @param value Option value
    /// @param options Options to update
    /// @return true if option was applied, false otherwise
    [[nodiscard]] bool applyConfigOption(const std::string& section,
                                        const std::string& key,
                                        const std::string& value,
                                        CompilerOptions& options);
    
    /// Convert string to enum value with error checking
    /// @tparam T Enum type
    /// @param value String value to convert
    /// @param enumMap Map of string to enum values
    /// @param optionName Name of option for error reporting
    /// @return Optional enum value
    template<typename T>
    [[nodiscard]] std::optional<T> parseEnumValue(const std::string& value,
                                                 const std::unordered_map<std::string, T>& enumMap,
                                                 const std::string& optionName);
    
    /// Parse file size with units (KB, MB, GB)
    /// @param value String value with optional units
    /// @param optionName Name of option for error reporting
    /// @return Size in bytes, or 0 on error
    [[nodiscard]] std::size_t parseFileSize(const std::string& value,
                                          const std::string& optionName);
    
    /// Parse boolean value from string
    /// @param value String representation of boolean
    /// @param optionName Name of option for error reporting
    /// @return Boolean value, or nullopt on error
    [[nodiscard]] std::optional<bool> parseBoolValue(const std::string& value,
                                                    const std::string& optionName);
    
    /// Validate file path and report errors
    /// @param path Path to validate
    /// @param optionName Name of option for error reporting
    /// @param mustExist Whether path must exist
    /// @return true if path is valid, false otherwise
    [[nodiscard]] bool validatePath(const std::filesystem::path& path,
                                  const std::string& optionName,
                                  bool mustExist = false);
    
    /// Generate flat option list without categories
    /// @param cmd Current command context
    /// @param includeCommandSpecific Whether to include command-specific options
    /// @return Formatted help string
    [[nodiscard]] std::string generateFlatOptionList(Command cmd,
                                                     bool includeCommandSpecific) const;
    
    /// Format option for help display
    /// @param def Option definition
    /// @param width Width to align to
    /// @return Formatted option string
    [[nodiscard]] std::string formatOptionForHelp(const OptionDef& def, std::size_t width) const;
    
    /// Calculate the width of an option string
    /// @param def Option definition
    /// @return Width of the option string
    [[nodiscard]] std::size_t calculateOptionWidth(const OptionDef& def) const;
    
    /// Calculate edit distance for option suggestions
    /// @param a First string
    /// @param b Second string
    /// @return Edit distance
    [[nodiscard]] std::size_t calculateEditDistance(std::string_view a, 
                                                   std::string_view b) const;
};

/// Utility functions for option parsing

/// Convert command enum to string
/// @param cmd Command to convert
/// @return String representation
[[nodiscard]] std::string commandToString(Command cmd);

/// Convert string to command enum
/// @param str String to convert
/// @return Command enum value, or nullopt if invalid
[[nodiscard]] std::optional<Command> stringToCommand(std::string_view str);

/// Get default configuration search paths
/// @return Vector of paths to search for configuration files
[[nodiscard]] std::vector<std::filesystem::path> getDefaultConfigPaths();

/// Create option parser with default diagnostics
/// @return OptionParser instance with console diagnostic logger
[[nodiscard]] std::unique_ptr<OptionParser> createDefaultOptionParser();

} // namespace cxy::compiler