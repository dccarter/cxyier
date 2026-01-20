#include <cxy/compiler.hpp>
#include <cxy/diagnostics.hpp>
#include <iostream>
#include <sstream>

using namespace cxy;
using namespace cxy::compiler;

int main(int argc, char** argv) {
    // Create diagnostic logger (already has default console sink)
    DiagnosticLogger diagnostics;

    // Create option parser
    OptionParser parser(diagnostics);
    CompilerOptions options;

    // Initialize options with platform-specific defaults
    initializeOptions(options);

    // Parse command line arguments
    auto result = parser.parseCommandLine(argc, argv, options);

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

    // Check if we'll be using stdin before validation
    bool useStdin = options.inputFiles.empty() && std::cin.peek() != EOF;

    // Validate options (skip input file validation if using stdin)
    if (!useStdin && !parser.validateOptions(options)) {
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

    if (options.debug.verbose) {
        std::cout << "Compile mode options" << std::endl;
        // Show initialization results
        if (!options.system.includePaths.empty()) {
            std::cout << "  Include paths:" << std::endl;
            for (const auto& path : options.system.includePaths) {
                std::cout << "  -I" << path.string() << std::endl;
            }
        }

        if (!options.system.librarySearchPaths.empty()) {
            std::cout << "  Library search paths:" << std::endl;
            for (const auto& path : options.system.librarySearchPaths) {
                std::cout << "  -L" << path.string() << std::endl;
            }
        }

        if (options.system.sysroot) {
            std::cout << "  System root: " << options.system.sysroot->string() << std::endl;
        }

        if (options.system.buildDir) {
            std::cout << "  Build directory: " << options.system.buildDir->string() << std::endl;
        }

        if (options.system.libDir) {
            std::cout << "  Library directory: " << options.system.libDir->string() << std::endl;
        }

        if (options.system.pluginsDir) {
            std::cout << "  Plugins directory: " << options.system.pluginsDir->string() << std::endl;
        }
    }

    // Command-specific output
    switch (options.command) {
        case Command::Dev: {
            const auto* devOpts = options.getDevOptions();
            if (devOpts) {
                if (devOpts->printTokens) {
                    std::cout << " Token printing enabled" << std::endl;
                }
                if (devOpts->printAST) {
                    std::cout << " AST printing enabled" << std::endl;
                }
                if (devOpts->dumpMode != DumpMode::None) {
                    std::cout << " Dump mode enabled" << std::endl;
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

    // Save input files before moving options
    auto inputFiles = options.inputFiles;

    // Create compiler instance
    Compiler compiler(std::move(options));
    CompilationResult compileResult;

    // Determine what to compile
    if (inputFiles.empty()) {
        // We already checked stdin availability, so read from it
        if (useStdin) {
            // Read from stdin
            std::ostringstream buffer;
            buffer << std::cin.rdbuf();
            std::string source = buffer.str();

            if (!source.empty()) {
                diagnostics.info("Compiling from stdin...", Location{});
                compileResult = compiler.compileSource(source, "<stdin>");
            } else {
                std::cerr << "No input files specified and stdin is empty." << std::endl;
                return 1;
            }
        } else {
            std::cerr << "No input files specified and stdin is empty." << std::endl;
            return 1;
        }
    } else {
        // Compile input files
        for (const auto& inputFile : inputFiles) {
            diagnostics.info(std::format("Compiling: {}", inputFile.string()), Location{});
            compileResult = compiler.compileFile(inputFile);

            // For now, only compile the first file
            // TODO: Handle multiple files properly
            break;
        }
    }

    // Handle compilation result
    switch (compileResult.status) {
        case CompilationResult::Status::Success:
            diagnostics.info("Compilation successful!", Location{});


            break;

        case CompilationResult::Status::ParseError:
            std::cerr << "Parse error occurred." << std::endl;
            return 1;

        case CompilationResult::Status::SemanticError:
            std::cerr << "Semantic error occurred." << std::endl;
            return 1;

        case CompilationResult::Status::IOError:
            std::cerr << "I/O error occurred." << std::endl;
            return 1;

        case CompilationResult::Status::InternalError:
            std::cerr << "Internal compiler error occurred." << std::endl;
            return 1;
    }

    // Show diagnostic counts
    if (compileResult.errorCount > 0 || compileResult.warningCount > 0) {
        std::cout << "Diagnostics: " << compileResult.errorCount << " errors, "
                  << compileResult.warningCount << " warnings" << std::endl;
    }

    return compileResult.status == CompilationResult::Status::Success ? 0 : 1;
}
