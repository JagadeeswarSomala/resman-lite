#include "ResObjGenerator.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

namespace resman
{
    //──────────────────────────────
    // Setters
    //──────────────────────────────
    ResObjGenerator& ResObjGenerator::setClangPath(const std::string& path)
    {
        mClangPath = path;
        return *this;
    }

    ResObjGenerator& ResObjGenerator::setLlvmAsPath(const std::string& path)
    {
        mLlvmAsPath = path;
        return *this;
    }

    ResObjGenerator& ResObjGenerator::setLlvmLinkPath(const std::string& path)
    {
        mLlvmLinkPath = path;
        return *this;
    }

    ResObjGenerator& ResObjGenerator::setLlcPath(const std::string& path)
    {
        mLlcPath = path;
        return *this;
    }

    ResObjGenerator& ResObjGenerator::setInputCppDir(const std::string& dir)
    {
        mInputCppDir = dir;
        return *this;
    }

    ResObjGenerator& ResObjGenerator::setWorkingDir(const std::string& dir)
    {
        mWorkingDir = dir;
        return *this;
    }

    ResObjGenerator& ResObjGenerator::setOutputObj(const std::string& path)
    {
        mOutputObj = path;
        return *this;
    }

    ResObjGenerator& ResObjGenerator::setTargetTriple(const std::string& triple)
    {
        mTargetTriple = triple;
        return *this;
    }

    ResObjGenerator& ResObjGenerator::addIncludePath(const std::string& path)
    {
        if (!path.empty())
            mIncludePaths.push_back(path);

        return *this;
    }

    ResObjGenerator& ResObjGenerator::addIncludePath(const std::vector<std::string>& paths)
    {
        for (const auto& p : paths)
            if (!p.empty())
                mIncludePaths.push_back(p);

        return *this;
    }

    //──────────────────────────────
    // Helpers
    //──────────────────────────────
    bool ResObjGenerator::validateInputs() const
    {
        if (mInputCppDir.empty() || !fs::exists(mInputCppDir)) {
            std::cerr << "[ResObjGenerator] Error: input .cpp directory invalid: " << mInputCppDir << "\n";
            return false;
        }

        if (mWorkingDir.empty()) {
            std::cerr << "[ResObjGenerator] Error: working directory not set.\n";
            return false;
        }

        if (!fs::exists(mWorkingDir))
            fs::create_directories(mWorkingDir);

        if (mOutputObj.empty()) {
            std::cerr << "[ResObjGenerator] Error: output object path not set.\n";
            return false;
        }

        // Tool paths are optional if they exist in PATH
        return true;
    }

    std::vector<std::string> ResObjGenerator::collectCppFiles() const
    {
        std::vector<std::string> cppFiles;
        for (const auto& entry : fs::directory_iterator(mInputCppDir))
        {
            if (entry.path().extension() == ".cpp")
                cppFiles.push_back(entry.path().string());
        }

        if (cppFiles.empty())
            std::cerr << "[ResObjGenerator] Warning: no .cpp files found in " << mInputCppDir << "\n";

        return cppFiles;
    }

    std::string ResObjGenerator::quote(const std::string& path) const
    {
        if (path.find(' ') != std::string::npos)
            return "\"" + path + "\"";
        return path;
    }

    bool ResObjGenerator::invokeCmd(const std::string& cmd, const std::string& stepDesc) const
    {
        std::cout << "[ResObjGenerator] " << stepDesc << ":\n  " << cmd << "\n";
        int rc = std::system(cmd.c_str());
        if (rc != 0)
        {
            std::cerr << "[ResObjGenerator] Error: command failed (" << rc << "): " << stepDesc << "\n";
            return false;
        }
        return true;
    }

    //──────────────────────────────
    // Core Generation
    //──────────────────────────────
    bool ResObjGenerator::generateObjectFile() const
    {
        auto cppFiles = collectCppFiles();
        if (cppFiles.empty())
            return false;

        std::vector<std::string> bcFiles;

        std::string clangBin   = mClangPath.empty()   ? "clang++"   : mClangPath;
        std::string llvmAsBin  = mLlvmAsPath.empty()  ? "llvm-as"   : mLlvmAsPath;
        std::string llvmLinkBin= mLlvmLinkPath.empty()? "llvm-link" : mLlvmLinkPath;
        std::string llcBin     = mLlcPath.empty()     ? "llc"       : mLlcPath;

        // .cpp → .ll → .bc
        for (const auto& cpp : cppFiles)
        {
            fs::path stem = fs::path(cpp).stem();
            fs::path llFile = fs::path(mWorkingDir) / (stem.string() + ".ll");
            fs::path bcFile = fs::path(mWorkingDir) / (stem.string() + ".bc");

            std::ostringstream clangCmd;
            clangCmd << quote(clangBin)
                    << " -S -emit-llvm ";

            if (!mTargetTriple.empty())
                clangCmd << "--target=" << mTargetTriple << " ";

            // append include directories
            for (const auto& inc : mIncludePaths)
            {
                if (inc.find(' ') != std::string::npos)
                    clangCmd << "-I\"" << inc << "\" ";
                else
                    clangCmd << "-I" << inc << " ";
            }

            clangCmd << quote(cpp)
                    << " -o " << quote(llFile.string());

            if (!mTargetTriple.empty())
                clangCmd << " --target=" << mTargetTriple;

            if (!invokeCmd(clangCmd.str(), "Generating LLVM IR (.ll)"))
                return false;

            std::ostringstream asCmd;
            asCmd << quote(llvmAsBin)
                  << " " << quote(llFile.string())
                  << " -o " << quote(bcFile.string());

            if (!invokeCmd(asCmd.str(), "Assembling LLVM bitcode (.bc)"))
                return false;

            bcFiles.push_back(bcFile.string());
        }

        // link all .bc → all.bc
        fs::path mergedBC = fs::path(mWorkingDir) / "resman_lite_master_bit_code_file.bc";

        std::ostringstream linkCmd;
        linkCmd << quote(llvmLinkBin) << " ";
        for (const auto& bc : bcFiles)
            linkCmd << quote(bc) << " ";
        linkCmd << "-o " << quote(mergedBC.string());

        if (!invokeCmd(linkCmd.str(), "Linking all .bc files"))
            return false;

        // all.bc → .obj
        std::ostringstream llcCmd;
        llcCmd << quote(llcBin)
               << " -filetype=obj " << quote(mergedBC.string())
               << " -o " << quote(mOutputObj);

        if (!mTargetTriple.empty())
            llcCmd << " -mtriple=" << mTargetTriple;

        if (!invokeCmd(llcCmd.str(), "Generating final object (.obj/.o)"))
            return false;

        std::cout << "[ResObjGenerator] Successfully generated: " << mOutputObj << "\n";
        return true;
    }

    //──────────────────────────────
    // Entry Point
    //──────────────────────────────
    bool ResObjGenerator::run()
    {
        if (!validateInputs())
            return false;

        return generateObjectFile();
    }

} // namespace resman