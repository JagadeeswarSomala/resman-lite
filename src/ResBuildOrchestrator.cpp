#include "ResBuildOrchestrator.h"

#include <filesystem>
#include <iostream>
#include <cstdlib>

namespace fs = std::filesystem;

namespace resman
{
    ResBuildOrchestrator& ResBuildOrchestrator::setOptions(const BuildOptions& opts)
    {
        mOpts = opts;
        return *this;
    }

    bool ResBuildOrchestrator::prepareWorkingDir()
    {
        if (mOpts.workingDir.has_value() && !mOpts.workingDir->empty())
        {
            mActiveWorkingDir = *mOpts.workingDir;
            if (!fs::exists(mActiveWorkingDir))
                fs::create_directories(mActiveWorkingDir);
            mIsTempWorkingDir = false;
        }
        else
        {
            // create a temp working dir
            auto tempBase = fs::temp_directory_path() / "resman-lite-temp";
            fs::create_directories(tempBase);
            mActiveWorkingDir = tempBase.string();
            mIsTempWorkingDir = true;
        }
        std::cout << "[ResBuildOrchestrator] Using working dir: " << mActiveWorkingDir << "\n";
        return true;
    }

    void ResBuildOrchestrator::cleanupWorkingDir()
    {
        if (mIsTempWorkingDir)
        {
            try
            {
                fs::remove_all(mActiveWorkingDir);
                std::cout << "[ResBuildOrchestrator] Cleaned temporary working dir: " << mActiveWorkingDir << "\n";
            }
            catch (const std::exception& e)
            {
                std::cerr << "[ResBuildOrchestrator] Warning: Failed to delete temp dir: " << e.what() << "\n";
            }
        }
        else
        {
            std::cout << "[ResBuildOrchestrator] Keeping user-provided working dir: " << mActiveWorkingDir << "\n";
        }
    }

    bool ResBuildOrchestrator::run()
    {
        if (mOpts.resHeader.empty() || mOpts.outputObj.empty())
        {
            std::cerr << "[ResBuildOrchestrator] Missing mandatory options (--res-header, --obj-name)\n";
            return false;
        }

        prepareWorkingDir();

        // Parse header to get includes
        resman::ResHeaderParser headerParser;
        std::string jsonPath = mActiveWorkingDir + "/ast.json";

        headerParser.setHeaderFile(mOpts.resHeader)
                    .setOutputJson(jsonPath)
                    .addIncludePath(mOpts.includePaths)
                    .setClangPath(mOpts.clangPath);         

        if (!headerParser.run())
            return false;

        // Parse AST JSON
        resman::ResASTJsonParser astParser;

        astParser.setInputJson(jsonPath);
        if(!astParser.run())
            return false;

        std::vector<resman::ResourceInfo> resources = astParser.getResInfo();

        // Generate C++ sources
        resman::ResCppSrcGenerator cppGen;

        // Create cpp output dir
        std::string cppOutDir = mActiveWorkingDir + "/cpp_src_gen";
        if (!fs::exists(cppOutDir))
                fs::create_directories(cppOutDir);

        cppGen.setOutputCppDir(cppOutDir)
              .setResourceInfo(resources)
              .setResSearchPath(mOpts.resPaths);

        if (!cppGen.run())
            return false;

        // Generate .obj
        resman::ResObjGenerator objGen;

        std::string buildDir = mActiveWorkingDir + "/build";
        if (!fs::exists(buildDir))
                fs::create_directories(buildDir);

        objGen.setInputCppDir(cppOutDir)
              .setWorkingDir(buildDir)
              .setOutputObj(mOpts.outputObj)
              .setTargetTriple(mOpts.targetTriple)
              .addIncludePath(mOpts.includePaths)
              .setClangPath(mOpts.clangPath)
              .setLlvmAsPath(mOpts.llvmAsPath)
              .setLlvmLinkPath(mOpts.llvmLinkPath)
              .setLlcPath(mOpts.llcPath);

        if (!objGen.run())
            return false;

        return true;
    }

    ResBuildOrchestrator::~ResBuildOrchestrator()
    {
        cleanupWorkingDir();
    }

} // namespace resman
