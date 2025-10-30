#pragma once

#include <string>
#include <vector>
#include <optional>

namespace resman
{
    class ResHeaderParser
    {
    public:

        ResHeaderParser& setClangPath(const std::string& path);
        ResHeaderParser& setHeaderFile(const std::string& path);
        ResHeaderParser& setOutputJson(const std::string& path);
        ResHeaderParser& addIncludePath(const std::string& includeDir);
        ResHeaderParser& addIncludePath(const std::vector<std::string>& includeDir);
        ResHeaderParser& addDefine(const std::string& define);
        ResHeaderParser& addDefine(const std::vector<std::string>& define);

        // Execute the parsing step
        bool run();

        // get system command
        std::string getCommandLine() const;

    private:
        // Internal helpers
        bool validateInputs() const;
        bool invokeClangAST() const;

    private:
        std::string mClangPath;
        std::string mHeaderFile;
        std::string mOutputJson;

        std::vector<std::string> mIncludeDirs;
        std::vector<std::string> mDefines;
    };
}
