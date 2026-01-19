#include <cxy/compiler.hpp>
#include <cxy/diagnostics.hpp>
#include <iostream>

using namespace cxy;
using namespace cxy::compiler;

int main(int argc, char** argv) {
    // Create diagnostic logger with console sink
    DiagnosticLogger diagnostics;
    auto consoleSink = std::make_unique<ConsoleDiagnosticSink>();
    diagnostics.addSink(std::move(consoleSink));
    
    // Create option parser
    OptionParser parser(diagnostics);
    CompilerOptions options;
    
    // Initialize options with platform-specific defaults
    initializeOptions(options);
    
    // Parse command line arguments
    auto result = parser.parseCommandLine(argc, argv, options);
    
    // Update plugins directory if it's still default and build directory changed
    if (options.system.pluginsDir && options.system.buildDir) {
        std::string pluginsPath = options.system.pluginsDir->string();
        // Check if plugins directory is still the default pattern
        if (pluginsPath.find("/build/plugins") != std::string::npos) {
            options.system.pluginsDir = *options.system.buildDir / "plugins";
        }
    }
    
    switch (result) {
        case ParseResult::HelpRequested: {
            // Show general help for global -h, or command-specific help for command -h
            std::cout << parser.generateHelp("cxy", options.command) << std::endl;
            return 0;
        }
            
        case ParseResult::VersionRequested:
            std::cout << parser.generateVersion() << std::endl;
            return 0;
            
        case ParseResult::Error:
            std::cerr << "Error parsing command line options." << std::endl;
            return 1;
            
        case ParseResult::Success:
            break;
    }
    
    // Validate options
    if (!parser.validateOptions(options)) {
        std::cerr << "Option validation failed." << std::endl;
        return 1;
    }
    
    // Print parsed options for demonstration
    std::cout << "Cxy Compiler - Command: " << options.commandString() << std::endl;
    
    if (!options.inputFiles.empty()) {
        std::cout << "Input files:" << std::endl;
        for (const auto& file : options.inputFiles) {
            std::cout << "  " << file.string() << std::endl;
        }
    }
    
    if (options.outputFile) {
        std::cout << "Output file: " << options.outputFile->string() << std::endl;
    }
    
    if (!options.defines.empty()) {
        std::cout << "Defines:" << std::endl;
        for (const auto& define : options.defines) {
            std::cout << "  -D" << define << std::endl;
        }
    }
    
    if (options.debug.verbose) {
        // Show initialization results
        if (!options.system.includePaths.empty()) {
            std::cout << "Include paths:" << std::endl;
            for (const auto& path : options.system.includePaths) {
                std::cout << "  -I" << path.string() << std::endl;
            }
        }
        
        if (!options.system.librarySearchPaths.empty()) {
            std::cout << "Library search paths:" << std::endl;
            for (const auto& path : options.system.librarySearchPaths) {
                std::cout << "  -L" << path.string() << std::endl;
            }
        }
        
        if (options.system.sysroot) {
            std::cout << "System root: " << options.system.sysroot->string() << std::endl;
        }
        
        if (options.system.buildDir) {
            std::cout << "Build directory: " << options.system.buildDir->string() << std::endl;
        }
        
        if (options.system.libDir) {
            std::cout << "Library directory: " << options.system.libDir->string() << std::endl;
        }
        
        if (options.system.pluginsDir) {
            std::cout << "Plugins directory: " << options.system.pluginsDir->string() << std::endl;
        }
    }
    
    if (options.debug.verbose) {
        std::cout << "Verbose mode enabled" << std::endl;
    }
    
    // Command-specific output
    switch (options.command) {
        case Command::Dev: {
            const auto* devOpts = options.getDevOptions();
            if (devOpts) {
                if (devOpts->printTokens) {
                    std::cout << "Token printing enabled" << std::endl;
                }
                if (devOpts->printAST) {
                    std::cout << "AST printing enabled" << std::endl;
                }
                if (devOpts->dumpMode != DumpMode::None) {
                    std::cout << "Dump mode enabled" << std::endl;
                }
            }
            break;
        }
        case Command::Build: {
            const auto* buildOpts = options.getBuildOptions();
            if (buildOpts) {
                std::cout << "Build target: ";
                switch (buildOpts->target) {
                    case BuildTarget::Executable: std::cout << "executable"; break;
                    case BuildTarget::Shared: std::cout << "shared library"; break;
                    case BuildTarget::Static: std::cout << "static library"; break;
                }
                std::cout << std::endl;
            }
            break;
        }
        case Command::Test: {
            const auto* testOpts = options.getTestOptions();
            if (testOpts && testOpts->verbose) {
                std::cout << "Verbose test output enabled" << std::endl;
            }
            break;
        }
        case Command::Run:
        case Command::Check:
        case Command::Help:
        case Command::Version:
            break;
    }
    
    std::cout << "Compilation would proceed here..." << std::endl;
    
    return 0;
}
