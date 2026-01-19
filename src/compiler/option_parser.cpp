#include <cxy/compiler/option_parser.hpp>
#include <cxy/compiler/diagnostic_extensions.hpp>
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <sstream>

namespace cxy::compiler {

OptionParser::OptionParser(DiagnosticLogger& diagnostics) 
    : diagnostics_(diagnostics) {
    initializeOptionDefinitions();
}

ParseResult OptionParser::parseCommandLine(int& argc, char** argv, CompilerOptions& options) {
    if (argc < 2) {
        auto diag = makeDiagnosticExtensions(diagnostics_);
        diag.error("No command specified. Use 'cxy help' for usage information.");
        return ParseResult::Error;
    }

    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    // Check for global options before command parsing
    if (!args.empty() && (args[0] == "-V" || args[0] == "--version")) {
        return ParseResult::VersionRequested;
    }
    if (!args.empty() && (args[0] == "-h" || args[0] == "--help")) {
        options.setCommand(Command::Help);
        return ParseResult::HelpRequested;
    }

    // First argument should be the command
    auto cmd = stringToCommand(args[0]);
    if (!cmd) {
        auto diag = makeDiagnosticExtensions(diagnostics_);
        diag.error("Unknown command: '{}'. Use 'cxy help' for available commands.", args[0]);
        return ParseResult::Error;
    }

    // Set command first
    options.setCommand(*cmd);

    // Handle special commands immediately
    if (*cmd == Command::Help) {
        return ParseResult::HelpRequested;
    }
    if (*cmd == Command::Version) {
        return ParseResult::VersionRequested;
    }

    // Parse remaining arguments, checking for help and version first
    bool helpRequested = false;
    for (std::size_t i = 1; i < args.size(); ++i) {
        // Check for help or version option before other parsing
        if (args[i] == "--help" || args[i] == "-h") {
            helpRequested = true;
            break;
        }
        if (args[i] == "--version" || args[i] == "-V") {
            return ParseResult::VersionRequested;
        }
        if (!parseArgument(args[i], args, i, options)) {
            return ParseResult::Error;
        }
    }
    
    if (helpRequested) {
        return ParseResult::HelpRequested;
    }

    // Update argc/argv to remove parsed arguments
    argc = 1; // Keep program name
    
    return ParseResult::Success;
}

bool OptionParser::parseConfigFile(const std::filesystem::path& configPath, CompilerOptions& options) {
    auto diag = makeDiagnosticExtensions(diagnostics_);
    
    if (!std::filesystem::exists(configPath)) {
        diag.warning("Configuration file not found: {}", configPath.string());
        return false;
    }
    
    // For now, just return true as we haven't implemented TOML parsing yet
    diag.info("Configuration file parsing not yet implemented: {}", configPath.string());
    return true;
}

bool OptionParser::validateOptions(const CompilerOptions& options) {
    return options.validate(diagnostics_);
}

std::string OptionParser::generateHelp(const std::string& programName, Command cmd) const {
    std::ostringstream oss;
    
    if (cmd == Command::Help) {
        // General help
        oss << "Cxy Compiler\n\n";
        oss << "Usage:\n";
        oss << "    " << programName << " <COMMAND> [OPTIONS] [FILES]\n\n";
        oss << "Commands:\n";
        oss << "    dev       Development mode with debugging features\n";
        oss << "    build     Standard compilation/build mode\n";
        oss << "    test      Run internal tests\n";
        oss << "    run       Compile and execute (future)\n";
        oss << "    check     Syntax and semantic check only\n";
        oss << "    help      Show help information\n";
        oss << "    version   Show version information\n\n";
        
        // Add global flags
        oss << "Flags:\n";
        oss << generateFlatOptionList(cmd, false);
    } else {
        // Command-specific help
        oss << "Cxy Compiler - " << commandToString(cmd) << " command\n\n";
        oss << "Usage:\n";
        oss << "    " << programName << " " << commandToString(cmd) << " [OPTIONS] [FILES]\n\n";
        oss << "Flags:\n";
        
        // Show command-specific options first, then global options
        oss << generateFlatOptionList(cmd, true);
    }
    
    return oss.str();
}

std::string OptionParser::generateVersion() const {
    return "Cxy Compiler v0.1.0-Phase1\n";
}

std::optional<std::filesystem::path> OptionParser::findConfigFile() const {
    auto paths = getDefaultConfigPaths();
    
    for (const auto& path : paths) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    
    return std::nullopt;
}

void OptionParser::initializeOptionDefinitions() {
    // Basic shared options
    optionDefs_["output"] = OptionDef{
        "output", "o", "Output file", "<FILE>", OptionCategory::Output, 
        {}, true, false, ""
    };
    
    optionDefs_["verbose"] = OptionDef{
        "verbose", "v", "Verbose output", "", OptionCategory::Debug,
        {}, false, true, "false"
    };
    
    optionDefs_["help"] = OptionDef{
        "help", "h", "Show help", "", OptionCategory::System,
        {}, false, true, "false"
    };
    
    optionDefs_["version"] = OptionDef{
        "version", "V", "Show version", "", OptionCategory::System,
        {}, false, true, "false"
    };
    
    // Diagnostic options (shared across all commands)
    optionDefs_["warnings-as-errors"] = OptionDef{
        "warnings-as-errors", "W", "Treat warnings as errors", "", OptionCategory::Diagnostic,
        {}, false, true, "false"
    };
    
    optionDefs_["suppress-warnings"] = OptionDef{
        "suppress-warnings", "w", "Suppress all warnings", "", OptionCategory::Diagnostic,
        {}, false, true, "false"
    };
    
    optionDefs_["diagnostic-format"] = OptionDef{
        "diagnostic-format", "", "Diagnostic output format", "<FORMAT>", OptionCategory::Diagnostic,
        {}, true, false, "default"
    };
    
    optionDefs_["disable-warning"] = OptionDef{
        "disable-warning", "", "Disable specific warning", "<WARNING>", OptionCategory::Diagnostic,
        {}, true, false, ""
    };
    
    optionDefs_["enable-warning"] = OptionDef{
        "enable-warning", "", "Enable specific warning", "<WARNING>", OptionCategory::Diagnostic,
        {}, true, false, ""
    };
    
    optionDefs_["max-errors"] = OptionDef{
        "max-errors", "", "Maximum number of errors", "<NUM>", OptionCategory::Diagnostic,
        {}, true, false, "100"
    };
    
    optionDefs_["show-colors"] = OptionDef{
        "show-colors", "", "Use colored output", "", OptionCategory::Diagnostic,
        {}, false, true, "true"
    };
    
    optionDefs_["no-colors"] = OptionDef{
        "no-colors", "", "Disable colored output", "", OptionCategory::Diagnostic,
        {}, false, true, "false"
    };
    
    // System/Directory options (shared)
    optionDefs_["lib-dir"] = OptionDef{
        "lib-dir", "", "Standard library directory", "<DIR>", OptionCategory::System,
        {}, true, false, ""
    };
    
    optionDefs_["build-dir"] = OptionDef{
        "build-dir", "B", "Build output directory", "<DIR>", OptionCategory::System,
        {}, true, false, ""
    };
    
    optionDefs_["plugins-dir"] = OptionDef{
        "plugins-dir", "", "Compiler plugins directory", "<DIR>", OptionCategory::System,
        {}, true, false, ""
    };
    
    optionDefs_["sysroot"] = OptionDef{
        "sysroot", "", "System root directory", "<DIR>", OptionCategory::System,
        {}, true, false, ""
    };
    
    optionDefs_["include"] = OptionDef{
        "include", "I", "Add include directory", "<DIR>", OptionCategory::System,
        {}, true, false, ""
    };
    
    optionDefs_["library-path"] = OptionDef{
        "library-path", "L", "Add library search directory", "<DIR>", OptionCategory::System,
        {}, true, false, ""
    };
    
    // Memory options (shared)
    optionDefs_["arena-size"] = OptionDef{
        "arena-size", "", "Arena allocator size", "<SIZE>", OptionCategory::Memory,
        {}, true, false, "64MB"
    };
    
    optionDefs_["show-arena-stats"] = OptionDef{
        "show-arena-stats", "", "Show arena allocation statistics", "", OptionCategory::Memory,
        {}, false, true, "false"
    };
    
    optionDefs_["enable-memory-tracking"] = OptionDef{
        "enable-memory-tracking", "", "Enable memory usage tracking", "", OptionCategory::Memory,
        {}, false, true, "false"
    };
    
    optionDefs_["with-memory-trace"] = OptionDef{
        "with-memory-trace", "", "Enable memory tracing", "", OptionCategory::Memory,
        {}, false, true, "false"
    };
    
    // Feature options (shared)
    optionDefs_["enable-experimental"] = OptionDef{
        "enable-experimental", "", "Enable experimental features", "", OptionCategory::Feature,
        {}, false, true, "false"
    };
    
    optionDefs_["enable-feature"] = OptionDef{
        "enable-feature", "", "Enable specific feature", "<FEATURE>", OptionCategory::Feature,
        {}, true, false, ""
    };
    
    optionDefs_["disable-feature"] = OptionDef{
        "disable-feature", "", "Disable specific feature", "<FEATURE>", OptionCategory::Feature,
        {}, true, false, ""
    };
    
    optionDefs_["strict-number-literals"] = OptionDef{
        "strict-number-literals", "", "Enable strict number literal parsing", "", OptionCategory::Feature,
        {}, false, true, "false"
    };
    
    optionDefs_["unicode-identifiers"] = OptionDef{
        "unicode-identifiers", "", "Allow Unicode in identifiers", "", OptionCategory::Feature,
        {}, false, true, "true"
    };
    
    optionDefs_["no-unicode-identifiers"] = OptionDef{
        "no-unicode-identifiers", "", "Disallow Unicode in identifiers", "", OptionCategory::Feature,
        {}, false, true, "false"
    };
    
    // Preprocessor defines (shared)
    optionDefs_["define"] = OptionDef{
        "define", "D", "Define preprocessor macro", "<NAME[=VALUE]>", OptionCategory::Feature,
        {}, true, false, ""
    };
    
    // Debug options (dev-specific)
    optionDefs_["show-timing"] = OptionDef{
        "show-timing", "", "Show timing information", "", OptionCategory::Debug,
        {Command::Dev}, false, true, "false"
    };
    
    optionDefs_["show-memory-usage"] = OptionDef{
        "show-memory-usage", "", "Show memory usage statistics", "", OptionCategory::Debug,
        {Command::Dev}, false, true, "false"
    };
    
    optionDefs_["preserve-temps"] = OptionDef{
        "preserve-temps", "", "Keep temporary files", "", OptionCategory::Debug,
        {Command::Dev}, false, true, "false"
    };
    
    optionDefs_["debug-parser"] = OptionDef{
        "debug-parser", "", "Enable parser debugging", "", OptionCategory::Debug,
        {Command::Dev}, false, true, "false"
    };
    
    optionDefs_["debug-lexer"] = OptionDef{
        "debug-lexer", "", "Enable lexer debugging", "", OptionCategory::Debug,
        {Command::Dev}, false, true, "false"
    };
    
    // Compilation stage options (for dev, build, check commands)
    optionDefs_["stop-after"] = OptionDef{
        "stop-after", "", "Stop compilation after stage", "<STAGE>", OptionCategory::Output,
        {Command::Dev, Command::Build, Command::Check}, true, false, "codegen"
    };
    
    // Dev command specific options
    optionDefs_["print-tokens"] = OptionDef{
        "print-tokens", "", "Print tokenized output", "", OptionCategory::Debug,
        {Command::Dev}, false, true, "false"
    };
    
    optionDefs_["print-ast"] = OptionDef{
        "print-ast", "", "Print abstract syntax tree", "", OptionCategory::Debug,
        {Command::Dev}, false, true, "false"
    };
    
    optionDefs_["dump"] = OptionDef{
        "dump", "", "Dump mode", "<MODE>", OptionCategory::Debug,
        {Command::Dev}, true, false, "none"
    };
    
    optionDefs_["emit-debug-info"] = OptionDef{
        "emit-debug-info", "", "Include debug information", "", OptionCategory::Debug,
        {Command::Dev}, false, true, "false"
    };
    
    optionDefs_["clean-ast"] = OptionDef{
        "clean-ast", "", "Clean AST output (no metadata)", "", OptionCategory::Debug,
        {Command::Dev}, false, true, "false"
    };
    
    optionDefs_["with-location"] = OptionDef{
        "with-location", "", "Include source location info", "", OptionCategory::Debug,
        {Command::Dev}, false, true, "true"
    };
    
    optionDefs_["without-location"] = OptionDef{
        "without-location", "", "Exclude source location info", "", OptionCategory::Debug,
        {Command::Dev}, false, true, "false"
    };
    
    optionDefs_["without-attrs"] = OptionDef{
        "without-attrs", "", "Exclude attributes in output", "", OptionCategory::Debug,
        {Command::Dev}, false, true, "false"
    };
    
    optionDefs_["dump-file"] = OptionDef{
        "dump-file", "", "File to dump output to", "<FILE>", OptionCategory::Output,
        {Command::Dev}, true, false, ""
    };
    
    // Build command specific options
    optionDefs_["shared"] = OptionDef{
        "shared", "", "Build shared library", "", OptionCategory::Output,
        {Command::Build}, false, true, "false"
    };
    
    optionDefs_["static"] = OptionDef{
        "static", "", "Build static library", "", OptionCategory::Output,
        {Command::Build}, false, true, "false"
    };
    
    optionDefs_["executable"] = OptionDef{
        "executable", "", "Build executable (default)", "", OptionCategory::Output,
        {Command::Build}, false, true, "true"
    };
    
    optionDefs_["no-pie"] = OptionDef{
        "no-pie", "", "Disable position independent executable", "", OptionCategory::Output,
        {Command::Build}, false, true, "false"
    };
    
    // Test command specific options
    optionDefs_["filter"] = OptionDef{
        "filter", "", "Test name filter pattern", "<PATTERN>", OptionCategory::Input,
        {Command::Test}, true, false, ""
    };
    
    optionDefs_["stop-on-failure"] = OptionDef{
        "stop-on-failure", "", "Stop on first test failure", "", OptionCategory::System,
        {Command::Test}, false, true, "false"
    };
    
    optionDefs_["test-verbose"] = OptionDef{
        "test-verbose", "", "Verbose test output", "", OptionCategory::Debug,
        {Command::Test}, false, true, "false"
    };
}

bool OptionParser::parseArgument(std::string_view arg, const std::vector<std::string>& args, 
                                 std::size_t& index, CompilerOptions& options) {
    if (arg.starts_with("--")) {
        return parseLongOption(arg.substr(2), args, index, options);
    } else if (arg.starts_with("-") && arg.length() > 1) {
        return parseShortOption(arg.substr(1), args, index, options);
    } else {
        // Input file
        options.inputFiles.emplace_back(arg);
        return true;
    }
}

bool OptionParser::parseShortOption(std::string_view option, const std::vector<std::string>& args,
                                    std::size_t& index, CompilerOptions& options) {
    // Handle -I and -L style options with attached values (e.g., -I/usr/include)
    if (option.length() > 1) {
        std::string_view shortOpt = option.substr(0, 1);
        std::string value(option.substr(1));
        
        // Special handling for -D defines which can have = in them
        if (shortOpt == "D") {
            return applyOption("define", value, options);
        }
        
        for (const auto& [name, def] : optionDefs_) {
            if (def.shortName == shortOpt && def.requiresValue) {
                return applyOption(name, value, options);
            }
        }
    }
    
    // Handle single character options
    if (option.length() == 1) {
        for (const auto& [name, def] : optionDefs_) {
            if (def.shortName == option) {
                if (def.requiresValue) {
                    std::string value = getOptionValue(args, index, name);
                    if (value.empty()) return false;
                    return applyOption(name, value, options);
                } else {
                    return applyOption(name, "true", options);
                }
            }
        }
    }
    
    auto diag = makeDiagnosticExtensions(diagnostics_);
    diag.error("Unknown short option: '-{}'", option);
    return false;
}

bool OptionParser::parseLongOption(std::string_view option, const std::vector<std::string>& args,
                                   std::size_t& index, CompilerOptions& options) {
    std::string optionName;
    std::string value;
    
    // Check for --option=value format
    auto equalPos = option.find('=');
    if (equalPos != std::string_view::npos) {
        optionName = option.substr(0, equalPos);
        value = option.substr(equalPos + 1);
    } else {
        optionName = option;
    }
    
    // Find option definition
    auto it = optionDefs_.find(optionName);
    if (it == optionDefs_.end()) {
        auto diag = makeDiagnosticExtensions(diagnostics_);
        diag.error("Unknown option: '--{}'", optionName);
        return false;
    }
    
    const auto& def = it->second;
    
    // Check if option is valid for current command
    if (!def.validCommands.empty()) {
        bool validForCommand = std::find(def.validCommands.begin(), def.validCommands.end(), 
                                        options.command) != def.validCommands.end();
        if (!validForCommand) {
            auto diag = makeDiagnosticExtensions(diagnostics_);
            diag.error("Option '--{}' is not valid for '{}' command", optionName, 
                      commandToString(options.command));
            return false;
        }
    }
    
    // Get value if needed
    if (def.requiresValue && value.empty()) {
        value = getOptionValue(args, index, optionName);
        if (value.empty()) return false;
    } else if (!def.requiresValue && value.empty()) {
        value = "true";
    }
    
    return applyOption(optionName, value, options);
}

bool OptionParser::applyOption(const std::string& optionName, const std::string& value,
                               CompilerOptions& options) {
    auto diag = makeDiagnosticExtensions(diagnostics_);
    
    // Basic options
    if (optionName == "output") {
        options.outputFile = value;
    } else if (optionName == "verbose") {
        options.debug.verbose = (value == "true");
    } else if (optionName == "help") {
        return true; // Handled elsewhere
    } else if (optionName == "version") {
        return true; // Handled elsewhere
    
    // Diagnostic options (shared)
    } else if (optionName == "warnings-as-errors") {
        options.diagnostics.warningsAsErrors = (value == "true");
    } else if (optionName == "suppress-warnings") {
        options.diagnostics.suppressWarnings = (value == "true");
    } else if (optionName == "diagnostic-format") {
        if (value == "default") options.diagnostics.format = DiagnosticFormat::Default;
        else if (value == "json") options.diagnostics.format = DiagnosticFormat::Json;
        else if (value == "brief") options.diagnostics.format = DiagnosticFormat::Brief;
        else if (value == "verbose") options.diagnostics.format = DiagnosticFormat::Verbose;
        else {
            diag.error("Invalid diagnostic format: '{}'. Valid formats: default, json, brief, verbose", value);
            return false;
        }
    } else if (optionName == "disable-warning") {
        options.diagnostics.disableWarnings.push_back(value);
    } else if (optionName == "enable-warning") {
        options.diagnostics.enableWarnings.push_back(value);
    } else if (optionName == "max-errors") {
        try {
            options.diagnostics.maxErrors = std::stoul(value);
        } catch (const std::exception&) {
            diag.error("Invalid max errors value: '{}'", value);
            return false;
        }
    } else if (optionName == "show-colors") {
        options.diagnostics.showColors = true;
    } else if (optionName == "no-colors") {
        options.diagnostics.showColors = false;
    
    // System/Directory options
    } else if (optionName == "lib-dir") {
        options.system.libDir = value;
    } else if (optionName == "build-dir") {
        options.system.buildDir = value;
    } else if (optionName == "plugins-dir") {
        options.system.pluginsDir = value;
    } else if (optionName == "sysroot") {
        options.system.sysroot = value;
    } else if (optionName == "include") {
        options.system.includePaths.emplace_back(value);
    } else if (optionName == "library-path") {
        options.system.librarySearchPaths.emplace_back(value);
    
    // Memory options
    } else if (optionName == "arena-size") {
        options.memory.arenaSize = parseFileSize(value, optionName);
        if (options.memory.arenaSize == 0) return false;
    } else if (optionName == "show-arena-stats") {
        options.memory.showArenaStats = (value == "true");
    } else if (optionName == "enable-memory-tracking") {
        options.memory.enableMemoryTracking = (value == "true");
    } else if (optionName == "with-memory-trace") {
        options.memory.withMemoryTrace = (value == "true");
    
    // Feature options
    } else if (optionName == "enable-experimental") {
        options.features.enableExperimentalFeatures = (value == "true");
    } else if (optionName == "enable-feature") {
        options.features.enabledFeatures.push_back(value);
    } else if (optionName == "disable-feature") {
        options.features.disabledFeatures.push_back(value);
    } else if (optionName == "strict-number-literals") {
        options.features.strictNumberLiterals = (value == "true");
    } else if (optionName == "unicode-identifiers") {
        options.features.allowUnicodeIdentifiers = (value == "true");
    } else if (optionName == "no-unicode-identifiers") {
        options.features.allowUnicodeIdentifiers = false;
    
    // Preprocessor defines
    } else if (optionName == "define") {
        options.defines.push_back(value);
    
    // Debug options (dev-specific)
    } else if (optionName == "show-timing") {
        if (auto* devOpts = options.getDevOptions()) {
            options.debug.showTiming = (value == "true");
        }
    } else if (optionName == "show-memory-usage") {
        if (auto* devOpts = options.getDevOptions()) {
            options.debug.showMemoryUsage = (value == "true");
        }
    } else if (optionName == "preserve-temps") {
        if (auto* devOpts = options.getDevOptions()) {
            options.debug.preserveTemps = (value == "true");
        }
    } else if (optionName == "debug-parser") {
        if (auto* devOpts = options.getDevOptions()) {
            options.debug.debugParser = (value == "true");
        }
    } else if (optionName == "debug-lexer") {
        if (auto* devOpts = options.getDevOptions()) {
            options.debug.debugLexer = (value == "true");
        }
    
    // Compilation stage options
    } else if (optionName == "stop-after") {
        if (value == "lex") options.stopAfter = CompileStage::Lex;
        else if (value == "parse") options.stopAfter = CompileStage::Parse;
        else if (value == "semantic") options.stopAfter = CompileStage::Semantic;
        else if (value == "optimize") options.stopAfter = CompileStage::Optimize;
        else if (value == "codegen") options.stopAfter = CompileStage::Codegen;
        else {
            diag.error("Invalid compilation stage: '{}'. Valid stages: lex, parse, semantic, optimize, codegen", value);
            return false;
        }
    
    // Dev command specific options
    } else if (optionName == "print-tokens") {
        if (auto* devOpts = options.getDevOptions()) {
            devOpts->printTokens = (value == "true");
        }
    } else if (optionName == "print-ast") {
        if (auto* devOpts = options.getDevOptions()) {
            devOpts->printAST = (value == "true");
        }
    } else if (optionName == "dump") {
        if (auto* devOpts = options.getDevOptions()) {
            if (value == "tokens") devOpts->dumpMode = DumpMode::Tokens;
            else if (value == "ast") devOpts->dumpMode = DumpMode::AST;
            else if (value == "json") devOpts->dumpMode = DumpMode::ASTJson;
            else if (value == "debug") devOpts->dumpMode = DumpMode::ASTDebug;
            else if (value == "none") devOpts->dumpMode = DumpMode::None;
            else {
                diag.error("Invalid dump mode: '{}'. Valid modes: tokens, ast, json, debug, none", value);
                return false;
            }
        }
    } else if (optionName == "emit-debug-info") {
        if (auto* devOpts = options.getDevOptions()) {
            devOpts->emitDebugInfo = (value == "true");
        }
    } else if (optionName == "clean-ast") {
        if (auto* devOpts = options.getDevOptions()) {
            devOpts->cleanAST = (value == "true");
        }
    } else if (optionName == "with-location") {
        if (auto* devOpts = options.getDevOptions()) {
            devOpts->withLocation = (value == "true");
        }
    } else if (optionName == "without-location") {
        if (auto* devOpts = options.getDevOptions()) {
            devOpts->withLocation = false;
        }
    } else if (optionName == "without-attrs") {
        if (auto* devOpts = options.getDevOptions()) {
            devOpts->withoutAttrs = (value == "true");
        }
    } else if (optionName == "dump-file") {
        if (auto* devOpts = options.getDevOptions()) {
            devOpts->dumpFile = value;
        }
    
    // Build command specific options
    } else if (optionName == "shared") {
        if (auto* buildOpts = options.getBuildOptions()) {
            buildOpts->target = BuildTarget::Shared;
        }
    } else if (optionName == "static") {
        if (auto* buildOpts = options.getBuildOptions()) {
            buildOpts->target = BuildTarget::Static;
        }
    } else if (optionName == "executable") {
        if (auto* buildOpts = options.getBuildOptions()) {
            buildOpts->target = BuildTarget::Executable;
        }
    } else if (optionName == "no-pie") {
        if (auto* buildOpts = options.getBuildOptions()) {
            buildOpts->noPIE = (value == "true");
        }
    
    // Test command specific options
    } else if (optionName == "filter") {
        if (auto* testOpts = options.getTestOptions()) {
            testOpts->testFilters.push_back(value);
        }
    } else if (optionName == "stop-on-failure") {
        if (auto* testOpts = options.getTestOptions()) {
            testOpts->stopOnFirstFailure = (value == "true");
        }
    } else if (optionName == "test-verbose") {
        if (auto* testOpts = options.getTestOptions()) {
            testOpts->verbose = (value == "true");
        }
    
    } else {
        diag.warning("Option '{}' not yet implemented", optionName);
    }
    
    return true;
}

std::string OptionParser::getOptionValue(const std::vector<std::string>& args, std::size_t& index,
                                        const std::string& optionName) {
    if (index + 1 >= args.size()) {
        auto diag = makeDiagnosticExtensions(diagnostics_);
        diag.error("Option '--{}' requires a value", optionName);
        return "";
    }
    
    ++index;
    return args[index];
}

void OptionParser::reportUnknownOption(std::string_view option, const Location& location) {
    auto diag = makeDiagnosticExtensions(diagnostics_);
    auto suggestions = suggestSimilarOptions(option);
    
    if (suggestions.empty()) {
        diag.error("Unknown option: '{}'", option);
    } else {
        std::ostringstream oss;
        oss << "Unknown option: '" << option << "'. Did you mean: ";
        for (size_t i = 0; i < suggestions.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << "'" << suggestions[i] << "'";
        }
        oss << "?";
        diag.error(oss.str());
    }
}

std::vector<std::string> OptionParser::suggestSimilarOptions(std::string_view option) const {
    std::vector<std::string> suggestions;
    
    for (const auto& [name, def] : optionDefs_) {
        if (calculateEditDistance(option, name) <= 2) {
            suggestions.push_back(name);
        }
    }
    
    return suggestions;
}

std::size_t OptionParser::calculateEditDistance(std::string_view a, std::string_view b) const {
    if (a.empty()) return b.length();
    if (b.empty()) return a.length();
    
    std::vector<std::vector<std::size_t>> dp(a.length() + 1, 
                                            std::vector<std::size_t>(b.length() + 1));
    
    for (std::size_t i = 0; i <= a.length(); ++i) dp[i][0] = i;
    for (std::size_t j = 0; j <= b.length(); ++j) dp[0][j] = j;
    
    for (std::size_t i = 1; i <= a.length(); ++i) {
        for (std::size_t j = 1; j <= b.length(); ++j) {
            if (a[i-1] == b[j-1]) {
                dp[i][j] = dp[i-1][j-1];
            } else {
                dp[i][j] = 1 + std::min({dp[i-1][j], dp[i][j-1], dp[i-1][j-1]});
            }
        }
    }
    
    return dp[a.length()][b.length()];
}

std::size_t OptionParser::parseFileSize(const std::string& value, const std::string& optionName) {
    auto diag = makeDiagnosticExtensions(diagnostics_);
    
    if (value.empty()) {
        diag.error("Empty size value for option '{}'", optionName);
        return 0;
    }
    
    std::string numberPart;
    std::string suffixPart;
    
    // Split into number and suffix
    auto i = value.find_first_not_of("0123456789");
    if (i == std::string::npos) {
        numberPart = value;
    } else {
        numberPart = value.substr(0, i);
        suffixPart = value.substr(i);
        
        // Convert suffix to lowercase
        std::transform(suffixPart.begin(), suffixPart.end(), suffixPart.begin(), 
                       [](char c) { return std::tolower(c); });
    }
    
    // Parse number
    std::size_t number;
    try {
        number = std::stoull(numberPart);
    } catch (const std::exception&) {
        diag.error("Invalid size number '{}' for option '{}'", numberPart, optionName);
        return 0;
    }
    
    // Apply multiplier based on suffix
    std::size_t multiplier = 1;
    if (suffixPart.empty() || suffixPart == "b") {
        multiplier = 1;
    } else if (suffixPart == "kb" || suffixPart == "k") {
        multiplier = 1024;
    } else if (suffixPart == "mb" || suffixPart == "m") {
        multiplier = 1024 * 1024;
    } else if (suffixPart == "gb" || suffixPart == "g") {
        multiplier = 1024 * 1024 * 1024;
    } else {
        diag.error("Invalid size suffix '{}' for option '{}'. Valid suffixes: B, KB, MB, GB", 
                  suffixPart, optionName);
        return 0;
    }
    
    // Check for overflow
    if (number > std::numeric_limits<std::size_t>::max() / multiplier) {
        diag.error("Size value '{}' is too large for option '{}'", value, optionName);
        return 0;
    }
    
    return number * multiplier;
}

std::string OptionParser::generateFlatOptionList(Command cmd, bool includeCommandSpecific) const {
    std::ostringstream oss;
    std::vector<const OptionDef*> options;
    
    // Collect all relevant options
    for (const auto& [name, def] : optionDefs_) {
        bool isCommandSpecific = !def.validCommands.empty();
        bool isValidForCommand = def.validCommands.empty() || 
                                std::find(def.validCommands.begin(), def.validCommands.end(), cmd) != def.validCommands.end();
        
        if (cmd == Command::Help) {
            // For global help, only show global options
            if (!isCommandSpecific && isValidForCommand) {
                options.push_back(&def);
            }
        } else {
            // For command help, show command-specific first, then global
            if (includeCommandSpecific) {
                if (isValidForCommand) {
                    options.push_back(&def);
                }
            }
        }
    }
    
    if (options.empty()) {
        return "";
    }
    
    // Sort command-specific options first, then global options, then alphabetically
    std::sort(options.begin(), options.end(),
              [cmd](const OptionDef* a, const OptionDef* b) {
                  bool aIsSpecific = !a->validCommands.empty();
                  bool bIsSpecific = !b->validCommands.empty();
                  
                  if (aIsSpecific != bIsSpecific) {
                      return aIsSpecific > bIsSpecific; // Command-specific first
                  }
                  return a->longName < b->longName; // Then alphabetical
              });
    
    // Calculate maximum option width for proper alignment
    std::size_t maxOptionWidth = 0;
    for (const auto* def : options) {
        std::size_t optionWidth = calculateOptionWidth(*def);
        maxOptionWidth = std::max(maxOptionWidth, optionWidth);
    }
    
    // Add some padding to the maximum width
    maxOptionWidth += 4;
    
    // Generate option descriptions with proper alignment
    for (const auto* def : options) {
        oss << "    " << formatOptionForHelp(*def, maxOptionWidth) << "\n";
    }
    
    return oss.str();
}

std::string OptionParser::formatOptionForHelp(const OptionDef& def, std::size_t width) const {
    std::ostringstream oss;
    
    // Build option string
    if (!def.shortName.empty()) {
        oss << "-" << def.shortName << ", ";
    } else {
        oss << "    ";
    }
    
    oss << "--" << def.longName;
    
    if (def.requiresValue) {
        oss << " " << def.argumentName;
    }
    
    // Pad to specified width
    std::string optionStr = oss.str();
    if (optionStr.length() < width) {
        optionStr.append(width - optionStr.length(), ' ');
    } else {
        // If option is longer than width, add minimum spacing
        optionStr += "  ";
    }
    
    // Add description
    optionStr += def.description;
    
    return optionStr;
}

std::size_t OptionParser::calculateOptionWidth(const OptionDef& def) const {
    std::size_t width = 0;
    
    // Account for short option if present
    if (!def.shortName.empty()) {
        width += 4; // "-X, "
    } else {
        width += 4; // "    "
    }
    
    // Account for long option
    width += 2; // "--"
    width += def.longName.length();
    
    // Account for argument if present
    if (def.requiresValue) {
        width += 1; // space
        width += def.argumentName.length();
    }
    
    return width;
}

} // namespace cxy::compiler