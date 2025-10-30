#pragma once

#include <string>
#include <vector>
#include <optional>
#include "ResASTJsonParser.h" // for ResourceInfo

namespace resman
{
    class ResCppSrcGenerator
    {
    public:
        ResCppSrcGenerator& setOutputCppDir(const std::string& path);
        ResCppSrcGenerator& setResourceInfo(const std::vector<ResourceInfo>& resInfo);
        ResCppSrcGenerator& setResSearchPath(const std::string& resSearchPath);
        ResCppSrcGenerator& setResSearchPath(const std::vector<std::string>& resSearchPath);

        bool run();

    private:
        bool validateInputs() const;
        bool generateCppSource() const;

        std::string sanitizeIdentifier(const std::string& input) const;

    private:
        std::string mOutputCppDir;
        std::vector<std::string> mResSearchPaths;
        std::vector<ResourceInfo> mResInfo;
    };
}
