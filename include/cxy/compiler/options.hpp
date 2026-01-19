#pragma once

#include "options_types.hpp"
#include <cxy/diagnostics.hpp>
#include <filesystem>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace cxy::compiler {

/// Main compiler options class that holds all configuration
class CompilerOptions {
public:
    /// Default constructor with sensible defaults
    CompilerOptions() = default;
    
    /// Copy constructor
    CompilerOptions(const CompilerOptions&) = default;
    
    /// Move constructor
    CompilerOptions(CompilerOptions&&) = default;
    
    /// Copy assignment
    CompilerOptions& operator=(const CompilerOptions&) = default;
    
    /// Move assignment
    CompilerOptions& operator=(CompilerOptions&&) = default;
    
    /// Destructor
    ~CompilerOptions() = default;

    // Core command and file options
    Command command = Command::Build;                           ///< Primary command to execute
    std::vector<std::filesystem::path> inputFiles;             ///< Source files to process
    std::optional<std::filesystem::path> outputFile;           ///< Output file name
    std::optional<std::filesystem::path> outputDir;            ///< Output directory
    CompileStage stopAfter = CompileStage::Codegen;            ///< Compilation stage to stop after
    
    // Command-specific options using variant for type safety
    std::variant<DevOptions, BuildOptions, TestOptions> commandOptions;
    
    // Global option categories
    DebugOptions debug;                                         ///< Debug and development options
    DiagnosticOptions diagnostics;                             ///< Diagnostic reporting options
    FeatureOptions features;                                   ///< Language feature controls
    OptimizationOptions optimization;                         ///< Optimization settings
    SystemOptions system;                                     ///< System and environment options
    MemoryOptions memory;                                     ///< Memory management options
    
    // Additional fields for compatibility with original cxy design
    std::optional<std::filesystem::path> configFile;          ///< Configuration file path
    std::vector<std::string> defines;                         ///< Preprocessor defines
    std::string rest;                                          ///< Remaining unparsed arguments
    
    /// Get command-specific options of the specified type
    /// @tparam T The option type to retrieve (DevOptions, BuildOptions, TestOptions)
    /// @return Pointer to options if current command matches type, nullptr otherwise
    template<typename T>
    [[nodiscard]] T* getCommandOptions() {
        return std::get_if<T>(&commandOptions);
    }
    
    /// Get command-specific options of the specified type (const version)
    /// @tparam T The option type to retrieve (DevOptions, BuildOptions, TestOptions)
    /// @return Pointer to options if current command matches type, nullptr otherwise
    template<typename T>
    [[nodiscard]] const T* getCommandOptions() const {
        return std::get_if<T>(&commandOptions);
    }
    
    /// Get dev options (convenience method)
    /// @return Pointer to DevOptions if command is Dev, nullptr otherwise
    [[nodiscard]] DevOptions* getDevOptions() {
        return getCommandOptions<DevOptions>();
    }
    
    /// Get dev options (const convenience method)
    /// @return Pointer to DevOptions if command is Dev, nullptr otherwise
    [[nodiscard]] const DevOptions* getDevOptions() const {
        return getCommandOptions<DevOptions>();
    }
    
    /// Get build options (convenience method)
    /// @return Pointer to BuildOptions if command is Build, nullptr otherwise
    [[nodiscard]] BuildOptions* getBuildOptions() {
        return getCommandOptions<BuildOptions>();
    }
    
    /// Get build options (const convenience method)
    /// @return Pointer to BuildOptions if command is Build, nullptr otherwise
    [[nodiscard]] const BuildOptions* getBuildOptions() const {
        return getCommandOptions<BuildOptions>();
    }
    
    /// Get test options (convenience method)
    /// @return Pointer to TestOptions if command is Test, nullptr otherwise
    [[nodiscard]] TestOptions* getTestOptions() {
        return getCommandOptions<TestOptions>();
    }
    
    /// Get test options (const convenience method)
    /// @return Pointer to TestOptions if command is Test, nullptr otherwise
    [[nodiscard]] const TestOptions* getTestOptions() const {
        return getCommandOptions<TestOptions>();
    }
    
    /// Set command and initialize appropriate command-specific options
    /// @param cmd The command to set
    void setCommand(Command cmd) {
        command = cmd;
        switch (cmd) {
            case Command::Dev:
                commandOptions = DevOptions{};
                break;
            case Command::Build:
                commandOptions = BuildOptions{};
                break;
            case Command::Test:
                commandOptions = TestOptions{};
                break;
            case Command::Run:
            case Command::Check:
            case Command::Help:
            case Command::Version:
                // These commands don't have specific options yet
                commandOptions = BuildOptions{};  // Default to build options
                break;
        }
    }
    
    /// Check if the options are valid for the current command
    /// @param diagnostics Diagnostic logger for reporting validation errors
    /// @return true if options are valid, false otherwise
    [[nodiscard]] bool validate(DiagnosticLogger& diagnostics) const;
    
    /// Get a human-readable string representation of the current command
    /// @return String representation of the command
    [[nodiscard]] std::string commandString() const;
    
    /// Check if input files are required for the current command
    /// @return true if input files are required, false otherwise
    [[nodiscard]] bool requiresInputFiles() const {
        return command == Command::Dev || 
               command == Command::Build || 
               command == Command::Check ||
               command == Command::Run;
    }
    
    /// Check if the current command supports compilation stages
    /// @return true if command supports --stop-after, false otherwise
    [[nodiscard]] bool supportsCompileStages() const {
        return command == Command::Dev || 
               command == Command::Build ||
               command == Command::Check;
    }
    
    /// Get the effective output directory (outputDir or system.buildDir)
    /// @return The directory where output should be placed
    [[nodiscard]] std::filesystem::path getEffectiveOutputDir() const {
        if (outputDir) {
            return *outputDir;
        }
        if (system.buildDir) {
            return *system.buildDir;
        }
        return std::filesystem::current_path();
    }
    
    /// Get the effective library directory
    /// @return The directory where libraries should be found
    [[nodiscard]] std::optional<std::filesystem::path> getEffectiveLibDir() const {
        return system.libDir;
    }
    
    /// Get the effective plugins directory
    /// @return The directory where plugins should be found
    [[nodiscard]] std::optional<std::filesystem::path> getEffectivePluginsDir() const {
        return system.pluginsDir;
    }
};

/// Helper function to create default options for a specific command
/// @param cmd The command to create options for
/// @return CompilerOptions configured for the specified command
[[nodiscard]] CompilerOptions createDefaultOptions(Command cmd);

/// Helper function to merge two sets of compiler options
/// @param base Base options to start with
/// @param override Options that override the base options
/// @return Merged options
[[nodiscard]] CompilerOptions mergeOptions(const CompilerOptions& base, 
                                          const CompilerOptions& override);

/// Initialize compiler options with platform-specific defaults
/// @param options Options to initialize
void initializeOptions(CompilerOptions& options);

} // namespace cxy::compiler