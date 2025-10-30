#pragma once

#include <string>
#include <vector>

namespace resman
{
    class ResObjGenerator
    {
    public:
        ResObjGenerator& setClangPath(const std::string& path);
        ResObjGenerator& setLlvmAsPath(const std::string& path);
        ResObjGenerator& setLlvmLinkPath(const std::string& path);
        ResObjGenerator& setLlcPath(const std::string& path);
        
        ResObjGenerator& setInputCppDir(const std::string& dir);
        ResObjGenerator& setWorkingDir(const std::string& dir);
        ResObjGenerator& setOutputObj(const std::string& path);
        ResObjGenerator& setTargetTriple(const std::string& triple);

        // Include directories
        ResObjGenerator& addIncludePath(const std::string& path);
        ResObjGenerator& addIncludePath(const std::vector<std::string>& paths);

        bool run();

    private:
        bool validateInputs() const;
        bool generateObjectFile() const;
        bool invokeCmd(const std::string& cmd, const std::string& stepDesc) const;
        std::vector<std::string> collectCppFiles() const;
        std::string quote(const std::string& path) const;

    private:
        std::string mClangPath;
        std::string mLlvmAsPath;
        std::string mLlvmLinkPath;
        std::string mLlcPath;

        std::string mInputCppDir;   // where all resource .cpp files are
        std::string mWorkingDir;    // where .ll and .bc intermediates go
        std::string mOutputObj;     // final .obj/.o/.lib/.a
        std::string mTargetTriple;  // optional target triple

        std::vector<std::string> mIncludePaths;   // include directories
    };
}