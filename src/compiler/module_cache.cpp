#include <cxy/compiler/module_cache.hpp>
#include <cxy/diagnostics.hpp>
#include <filesystem>
#include <algorithm>

namespace cxy::compiler {

// CachedModule implementation
bool CachedModule::isValid() const {
    if (!std::filesystem::exists(canonicalPath)) {
        return false;
    }
    
    try {
        auto currentTime = std::filesystem::last_write_time(canonicalPath);
        return currentTime == timestamp;
    } catch (const std::filesystem::filesystem_error&) {
        return false;
    }
}

// ModuleCache implementation
bool ModuleCache::isCached(const std::filesystem::path& modulePath) const {
    auto it = cache_.find(modulePath);
    if (it == cache_.end()) {
        return false;
    }
    
    return it->second.isValid();
}

const ast::ASTNode* ModuleCache::getCachedModule(const std::filesystem::path& modulePath) const {
    auto it = cache_.find(modulePath);
    if (it == cache_.end()) {
        return nullptr;
    }
    
    if (!it->second.isValid()) {
        return nullptr;
    }
    
    return it->second.ast;
}

bool ModuleCache::cacheModule(const std::filesystem::path& modulePath,
                              ast::ASTNode* ast,
                              size_t errorCount,
                              size_t warningCount,
                              bool hasSemanticInfo) {
    CachedModule module;
    module.ast = ast;
    module.canonicalPath = modulePath;
    module.errorCount = errorCount;
    module.warningCount = warningCount;
    module.hasSemanticInfo = hasSemanticInfo;
    
    // Get file timestamp if file exists
    auto fileTime = getFileTime(modulePath);
    if (fileTime) {
        module.timestamp = *fileTime;
    } else {
        // If file doesn't exist, use current time
        try {
            module.timestamp = std::filesystem::file_time_type::clock::now();
        } catch (...) {
            // If we can't get any time, still cache the module
            module.timestamp = std::filesystem::file_time_type{};
        }
    }
    
    cache_[modulePath] = std::move(module);
    return true;
}

bool ModuleCache::removeModule(const std::filesystem::path& modulePath) {
    auto it = cache_.find(modulePath);
    if (it == cache_.end()) {
        return false;
    }
    
    cache_.erase(it);
    return true;
}

void ModuleCache::clear() {
    cache_.clear();
    importStack_.clear();
}

bool ModuleCache::beginImport(const std::filesystem::path& modulePath) {
    if (wouldCreateCycle(modulePath)) {
        return false;
    }
    
    importStack_.push_back(modulePath);
    return true;
}

void ModuleCache::endImport(const std::filesystem::path& modulePath) {
    auto it = std::find(importStack_.begin(), importStack_.end(), modulePath);
    if (it != importStack_.end()) {
        importStack_.erase(it);
    }
}

bool ModuleCache::wouldCreateCycle(const std::filesystem::path& modulePath) const {
    return std::find(importStack_.begin(), importStack_.end(), modulePath) != importStack_.end();
}

std::vector<std::filesystem::path> ModuleCache::getImportStack() const {
    return importStack_;
}

bool ModuleCache::invalidateIfModified(const std::filesystem::path& modulePath) {
    auto it = cache_.find(modulePath);
    if (it == cache_.end()) {
        return false;
    }
    
    if (!it->second.isValid()) {
        cache_.erase(it);
        return true;
    }
    
    return false;
}

size_t ModuleCache::invalidateModified() {
    size_t invalidated = 0;
    
    auto it = cache_.begin();
    while (it != cache_.end()) {
        if (!it->second.isValid()) {
            it = cache_.erase(it);
            ++invalidated;
        } else {
            ++it;
        }
    }
    
    return invalidated;
}

const CachedModule* ModuleCache::getModuleInfo(const std::filesystem::path& modulePath) const {
    auto it = cache_.find(modulePath);
    if (it == cache_.end()) {
        return nullptr;
    }
    
    return &it->second;
}

bool ModuleCache::allModulesHaveSemanticInfo() const {
    return std::all_of(cache_.begin(), cache_.end(),
                      [](const auto& pair) {
                          return pair.second.hasSemanticInfo;
                      });
}

std::optional<std::filesystem::file_time_type> ModuleCache::getFileTime(
    const std::filesystem::path& path) const {
    try {
        if (std::filesystem::exists(path)) {
            return std::filesystem::last_write_time(path);
        }
    } catch (const std::filesystem::filesystem_error&) {
        // File system error, treat as file doesn't exist
    }
    
    return std::nullopt;
}

// ImportGuard implementation
ImportGuard::ImportGuard(ModuleCache& cache, 
                         const std::filesystem::path& modulePath,
                         DiagnosticLogger& diagnostics)
    : cache_(cache), modulePath_(modulePath), isValid_(false), needsCleanup_(false) {
    
    if (cache_.wouldCreateCycle(modulePath)) {
        isValid_ = false;
        needsCleanup_ = false;
        
        // Report circular dependency error
        auto importStack = cache_.getImportStack();
        std::string cycleInfo = "Circular import detected: ";
        for (size_t i = 0; i < importStack.size(); ++i) {
            if (i > 0) cycleInfo += " -> ";
            cycleInfo += importStack[i].string();
        }
        cycleInfo += " -> " + modulePath.string();
        
        Location location{modulePath.string(), Position{1, 1, 0}};
        diagnostics.error(cycleInfo, location);
    } else {
        isValid_ = cache_.beginImport(modulePath);
        needsCleanup_ = isValid_;
    }
}

ImportGuard::~ImportGuard() {
    if (needsCleanup_) {
        cache_.endImport(modulePath_);
    }
}

} // namespace cxy::compiler