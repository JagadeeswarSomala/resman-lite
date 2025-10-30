#pragma once

#include <string>
#include <vector>
#include <optional>
#include "ResHeaderParser.h"
#include "ResASTJsonParser.h"
#include "ResCppSrcGenerator.h"
#include "ResObjGenerator.h"

namespace resman
{
    struct BuildOptions
    {
        std::string resHeader;                     // --res-header
        std::string outputObj;                     // --obj-name
        std::vector<std::string> includePaths;     // -I
        std::vector<std::string> resPaths;         // -R
        std::string targetTriple;                  // --mtriple
        std::optional<std::string> workingDir;     // optional

        // LLVM tool paths
        std::string clangPath    = "clang++";       // --clang-path
        std::string llvmAsPath   = "llvm-as";       // --llvm-as-path
        std::string llvmLinkPath = "llvm-link";     // --llvm-link-path
        std::string llcPath      = "llc";           // --llc-path
    };

    class ResBuildOrchestrator
    {
    public:
        ResBuildOrchestrator() = default;
        ~ResBuildOrchestrator();
        
        ResBuildOrchestrator& setOptions(const BuildOptions& opts);
        bool run();

    private:
        bool prepareWorkingDir();
        void cleanupWorkingDir();

    private:
        BuildOptions mOpts;
        std::string mActiveWorkingDir;
        bool mIsTempWorkingDir = false;
    };
}