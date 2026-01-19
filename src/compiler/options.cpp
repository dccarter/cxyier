#include <cxy/compiler/options.hpp>
#include <cxy/compiler/diagnostic_extensions.hpp>
#include <cxy/diagnostics.hpp>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

namespace cxy::compiler {

bool CompilerOptions::validate(DiagnosticLogger& diagnostics) const {
    auto diag = makeDiagnosticExtensions(diagnostics);
    bool isValid = true;
    
    // Check if input files are required for the current command
    if (requiresInputFiles() && inputFiles.empty()) {
        diag.error("No input files specified for {} command", commandString());
        isValid = false;
    }
    
    // Validate that input files exist (if specified)
    for (const auto& inputFile : inputFiles) {
        if (!std::filesystem::exists(inputFile)) {
            diag.error("Input file does not exist: {}", inputFile.string());
            isValid = false;
        }
        
        if (std::filesystem::is_directory(inputFile)) {
            diag.error("Input file is a directory: {}", inputFile.string());
            isValid = false;
        }
    }
    
    // Validate output directory if specified
    if (outputDir && !outputDir->empty()) {
        if (std::filesystem::exists(*outputDir) && !std::filesystem::is_directory(*outputDir)) {
            diag.error("Output directory path exists but is not a directory: {}", 
                            outputDir->string());
            isValid = false;
        }
    }
    
    // Validate system directories if specified
    if (system.libDir && !system.libDir->empty()) {
        if (!std::filesystem::exists(*system.libDir)) {
            diag.warning("Library directory does not exist: {}", system.libDir->string());
        } else if (!std::filesystem::is_directory(*system.libDir)) {
            diag.error("Library directory path is not a directory: {}", 
                            system.libDir->string());
            isValid = false;
        }
    }
    
    if (system.buildDir && !system.buildDir->empty()) {
        if (std::filesystem::exists(*system.buildDir) && !std::filesystem::is_directory(*system.buildDir)) {
            diag.error("Build directory path exists but is not a directory: {}", 
                            system.buildDir->string());
            isValid = false;
        }
    }
    
    if (system.pluginsDir && !system.pluginsDir->empty()) {
        if (std::filesystem::exists(*system.pluginsDir) && !std::filesystem::is_directory(*system.pluginsDir)) {
            diag.error("Plugins directory path exists but is not a directory: {}", 
                            system.pluginsDir->string());
            isValid = false;
        }
    }
    
    // Validate include paths
    for (const auto& includePath : system.includePaths) {
        if (!std::filesystem::exists(includePath)) {
            diag.warning("Include directory does not exist: {}", includePath.string());
        } else if (!std::filesystem::is_directory(includePath)) {
            diag.error("Include path is not a directory: {}", includePath.string());
            isValid = false;
        }
    }
    
    // Validate library search paths
    for (const auto& libPath : system.librarySearchPaths) {
        if (!std::filesystem::exists(libPath)) {
            diag.warning("Library search directory does not exist: {}", libPath.string());
        } else if (!std::filesystem::is_directory(libPath)) {
            diag.error("Library search path is not a directory: {}", libPath.string());
            isValid = false;
        }
    }
    
    // Validate compile stage compatibility with command
    if (!supportsCompileStages() && stopAfter != CompileStage::Codegen) {
        diag.warning("--stop-after option has no effect with {} command", commandString());
    }
    
    // Validate command-specific options
    switch (command) {
        case Command::Dev: {
            const auto* devOpts = getDevOptions();
            if (devOpts && devOpts->dumpFile) {
                const auto& dumpFile = *devOpts->dumpFile;
                if (std::filesystem::exists(dumpFile) && std::filesystem::is_directory(dumpFile)) {
                    diag.error("Dump file path is a directory: {}", dumpFile.string());
                    isValid = false;
                }
            }
            
            // Check for conflicting dump options
            if (devOpts && devOpts->dumpMode != DumpMode::None && stopAfter == CompileStage::Lex) {
                if (devOpts->dumpMode == DumpMode::AST || devOpts->dumpMode == DumpMode::ASTJson || 
                    devOpts->dumpMode == DumpMode::ASTDebug) {
                    diag.warning("Cannot dump AST when stopping after lexical analysis, "
                                      "changing dump mode to Tokens");
                }
            }
            break;
        }
        
        case Command::Build: {
            const auto* buildOpts = getBuildOptions();
            if (buildOpts) {
                // Validate build target consistency
                int targetCount = 0;
                if (buildOpts->target == BuildTarget::Executable) targetCount++;
                if (buildOpts->target == BuildTarget::Shared) targetCount++;
                if (buildOpts->target == BuildTarget::Static) targetCount++;
                
                // This validation is inherent in enum, but we could add future conflicts here
            }
            break;
        }
        
        case Command::Test: {
            // Test command validation would go here
            break;
        }
        
        case Command::Run:
        case Command::Check:
        case Command::Help:
        case Command::Version:
            // These commands have minimal validation requirements
            break;
    }
    
    // Validate memory options
    if (memory.arenaSize == 0) {
        diag.error("Arena size must be greater than zero");
        isValid = false;
    }
    
    if (memory.arenaSize < 1024) {
        diag.warning("Arena size is very small ({}), performance may be degraded", 
                          memory.arenaSize);
    }
    
    // Validate diagnostic options
    if (this->diagnostics.maxErrors == 0) {
        diag.warning("Maximum error count is zero, compilation will stop on first error");
    }
    
    return isValid;
}

std::string CompilerOptions::commandString() const {
    switch (command) {
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

CompilerOptions createDefaultOptions(Command cmd) {
    CompilerOptions options;
    options.setCommand(cmd);
    
    // Set command-specific defaults
    switch (cmd) {
        case Command::Dev:
            options.debug.verbose = true;
            options.debug.showTiming = true;
            if (auto* devOpts = options.getDevOptions()) {
                devOpts->withLocation = true;
                devOpts->emitDebugInfo = true;
            }
            break;
            
        case Command::Build:
            options.optimization.level = OptimizationLevel::Basic;
            if (auto* buildOpts = options.getBuildOptions()) {
                buildOpts->target = BuildTarget::Executable;
            }
            break;
            
        case Command::Test:
            options.debug.verbose = false;
            options.diagnostics.suppressWarnings = true;
            if (auto* testOpts = options.getTestOptions()) {
                testOpts->verbose = false;
            }
            break;
            
        case Command::Run:
            options.optimization.level = OptimizationLevel::None;
            options.debug.preserveTemps = false;
            break;
            
        case Command::Check:
            options.stopAfter = CompileStage::Semantic;
            options.diagnostics.warningsAsErrors = false;
            break;
            
        case Command::Help:
        case Command::Version:
            // No specific defaults needed
            break;
    }
    
    return options;
}

CompilerOptions mergeOptions(const CompilerOptions& base, const CompilerOptions& override) {
    CompilerOptions result = base;
    
    // Merge basic options
    if (!override.inputFiles.empty()) {
        result.inputFiles = override.inputFiles;
    }
    
    if (override.outputFile) {
        result.outputFile = override.outputFile;
    }
    
    if (override.outputDir) {
        result.outputDir = override.outputDir;
    }
    
    if (override.command != Command::Build || base.command == Command::Build) {
        result.command = override.command;
        result.commandOptions = override.commandOptions;
    }
    
    if (override.stopAfter != CompileStage::Codegen) {
        result.stopAfter = override.stopAfter;
    }
    
    // Merge debug options (override non-default values)
    if (override.debug.verbose) result.debug.verbose = true;
    if (override.debug.showTiming) result.debug.showTiming = true;
    if (override.debug.showMemoryUsage) result.debug.showMemoryUsage = true;
    if (override.debug.preserveTemps) result.debug.preserveTemps = true;
    if (override.debug.debugParser) result.debug.debugParser = true;
    if (override.debug.debugLexer) result.debug.debugLexer = true;
    
    // Merge diagnostic options
    if (override.diagnostics.warningsAsErrors) result.diagnostics.warningsAsErrors = true;
    if (override.diagnostics.suppressWarnings) result.diagnostics.suppressWarnings = true;
    if (!override.diagnostics.disableWarnings.empty()) {
        result.diagnostics.disableWarnings.insert(
            result.diagnostics.disableWarnings.end(),
            override.diagnostics.disableWarnings.begin(),
            override.diagnostics.disableWarnings.end()
        );
    }
    if (!override.diagnostics.enableWarnings.empty()) {
        result.diagnostics.enableWarnings.insert(
            result.diagnostics.enableWarnings.end(),
            override.diagnostics.enableWarnings.begin(),
            override.diagnostics.enableWarnings.end()
        );
    }
    if (override.diagnostics.format != DiagnosticFormat::Default) {
        result.diagnostics.format = override.diagnostics.format;
    }
    if (!override.diagnostics.showColors) result.diagnostics.showColors = false;
    if (override.diagnostics.maxErrors != 100) result.diagnostics.maxErrors = override.diagnostics.maxErrors;
    
    // Merge feature options
    if (override.features.enableExperimentalFeatures) result.features.enableExperimentalFeatures = true;
    if (override.features.strictNumberLiterals) result.features.strictNumberLiterals = true;
    if (!override.features.allowUnicodeIdentifiers) result.features.allowUnicodeIdentifiers = false;
    if (!override.features.enabledFeatures.empty()) {
        result.features.enabledFeatures.insert(
            result.features.enabledFeatures.end(),
            override.features.enabledFeatures.begin(),
            override.features.enabledFeatures.end()
        );
    }
    if (!override.features.disabledFeatures.empty()) {
        result.features.disabledFeatures.insert(
            result.features.disabledFeatures.end(),
            override.features.disabledFeatures.begin(),
            override.features.disabledFeatures.end()
        );
    }
    
    // Merge optimization options
    if (override.optimization.level != OptimizationLevel::None) {
        result.optimization.level = override.optimization.level;
    }
    if (override.optimization.debugInfo) result.optimization.debugInfo = true;
    if (!override.optimization.passes.empty()) {
        result.optimization.passes = override.optimization.passes;
    }
    if (override.optimization.debugPassManager) result.optimization.debugPassManager = true;
    
    // Merge system options
    if (override.system.stdlib) result.system.stdlib = override.system.stdlib;
    if (override.system.targetTriple) result.system.targetTriple = override.system.targetTriple;
    if (override.system.sysroot) result.system.sysroot = override.system.sysroot;
    if (override.system.libDir) result.system.libDir = override.system.libDir;
    if (override.system.buildDir) result.system.buildDir = override.system.buildDir;
    if (override.system.pluginsDir) result.system.pluginsDir = override.system.pluginsDir;
    if (override.system.operatingSystem) result.system.operatingSystem = override.system.operatingSystem;
    if (override.system.buildPlugin) result.system.buildPlugin = true;
    
    // Merge search paths (append, don't replace)
    if (!override.system.includePaths.empty()) {
        result.system.includePaths.insert(
            result.system.includePaths.end(),
            override.system.includePaths.begin(),
            override.system.includePaths.end()
        );
    }
    if (!override.system.librarySearchPaths.empty()) {
        result.system.librarySearchPaths.insert(
            result.system.librarySearchPaths.end(),
            override.system.librarySearchPaths.begin(),
            override.system.librarySearchPaths.end()
        );
    }
    if (!override.system.frameworkSearchPaths.empty()) {
        result.system.frameworkSearchPaths.insert(
            result.system.frameworkSearchPaths.end(),
            override.system.frameworkSearchPaths.begin(),
            override.system.frameworkSearchPaths.end()
        );
    }
    
    // Merge memory options
    if (override.memory.arenaSize != 64 * 1024 * 1024) {
        result.memory.arenaSize = override.memory.arenaSize;
    }
    if (override.memory.enableMemoryTracking) result.memory.enableMemoryTracking = true;
    if (override.memory.showArenaStats) result.memory.showArenaStats = true;
    if (override.memory.withMemoryTrace) result.memory.withMemoryTrace = true;
    
    // Merge other options
    if (override.configFile) result.configFile = override.configFile;
    if (!override.defines.empty()) {
        result.defines.insert(result.defines.end(), override.defines.begin(), override.defines.end());
    }
    if (!override.rest.empty()) result.rest = override.rest;
    
    return result;
}

// Helper function declaration for platform-specific operations
#ifdef __APPLE__
std::string getSDKPath();
#endif

void initializeOptions(CompilerOptions& options) {
    // Set up environment-based defaults
    if (const char* cxyOs = std::getenv("CXY_OS")) {
        options.system.operatingSystem = cxyOs;
    }
    
    // Platform-specific initialization
#ifdef __APPLE__
    // macOS platform defines
    options.defines.push_back("MACOS=1");
    options.defines.push_back("UNIX=1");
    
    // Add macOS system root if available
    std::string sdkPath = getSDKPath();
    if (!sdkPath.empty()) {
        options.system.sysroot = sdkPath;
        
        // Add SDK include path
        std::filesystem::path sdkInclude = std::filesystem::path(sdkPath) / "usr" / "include";
        if (std::filesystem::exists(sdkInclude)) {
            options.system.includePaths.push_back(sdkInclude);
        }
    }
    
#elif defined(__linux__)
    // Linux platform defines
    options.defines.push_back("LINUX=1");
    options.defines.push_back("UNIX=1");
    
    // Add common Linux include paths
    std::vector<std::string> commonIncludePaths = {
        "/usr/include",
        "/usr/local/include",
        "/usr/include/x86_64-linux-gnu"
    };
    
    for (const auto& path : commonIncludePaths) {
        if (std::filesystem::exists(path)) {
            options.system.includePaths.push_back(path);
        }
    }
    
    // Set OS-specific define if CXY_OS is set
    if (options.system.operatingSystem) {
        options.defines.push_back(*options.system.operatingSystem + "=1");
    }
    
#elif defined(_WIN32)
    // Windows platform defines
    options.defines.push_back("WINDOWS=1");
    
    // Add Windows-specific include paths if available
    if (const char* vsInstallDir = std::getenv("VCToolsInstallDir")) {
        std::filesystem::path vcInclude = std::filesystem::path(vsInstallDir) / "include";
        if (std::filesystem::exists(vcInclude)) {
            options.system.includePaths.push_back(vcInclude);
        }
    }
    
#else
    // Generic Unix-like system
    options.defines.push_back("UNIX=1");
    
    // Set OS-specific define if CXY_OS is set
    if (options.system.operatingSystem) {
        options.defines.push_back(*options.system.operatingSystem + "=1");
    }
#endif

    // Common library search paths
    std::vector<std::string> commonLibPaths = {
        "/usr/lib",
        "/usr/local/lib",
        "/lib"
    };
    
    for (const auto& path : commonLibPaths) {
        if (std::filesystem::exists(path)) {
            options.system.librarySearchPaths.push_back(path);
        }
    }
    
    // Set default build directory if not specified
    if (!options.system.buildDir) {
        options.system.buildDir = std::filesystem::current_path() / "build";
    }
    
    // Set up standard library directory from environment
    if (!options.system.libDir) {
        if (const char* cxyStdlibDir = std::getenv("CXY_STDLIB_DIR")) {
            options.system.libDir = cxyStdlibDir;
        } else if (const char* cxyRoot = std::getenv("CXY_ROOT")) {
            options.system.libDir = std::filesystem::path(cxyRoot) / "lib" / "cxy" / "std";
        }
    }
    
    // Set up plugins directory
    if (!options.system.pluginsDir) {
        if (options.system.buildDir) {
            options.system.pluginsDir = *options.system.buildDir / "plugins";
        } else {
            options.system.pluginsDir = "./plugins";
        }
    }
}

// Helper function to get macOS SDK path
#ifdef __APPLE__
std::string getSDKPath() {
    std::string result;
    FILE* pipe = popen("xcrun --show-sdk-path 2>/dev/null", "r");
    if (pipe) {
        char buffer[512];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            result = buffer;
            // Remove trailing newline
            if (!result.empty() && result.back() == '\n') {
                result.pop_back();
            }
        }
        pclose(pipe);
    }
    return result;
}
#endif

} // namespace cxy::compiler