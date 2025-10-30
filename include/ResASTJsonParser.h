#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp> // we’ll use this for JSON parsing

namespace resman
{
    struct ResourceInfo
    {
        std::string resName;
        std::string resType;
        std::string resFilepath;
    };

    class ResASTJsonParser
    {
    public:
        ResASTJsonParser& setInputJson(const std::string& path);
        ResASTJsonParser& setOutputJson(const std::string& path); // optional (for extracted data)
        bool run();

        // getters
        const std::vector<ResourceInfo>& getResources() const noexcept { return mResources; }

        // for debugging
        void printSummary() const;

    private:
        bool validateInputs() const;
        bool parseASTJson();
        void extractResources(const nlohmann::json& node);

    private:
        std::string mInputJson;
        std::string mOutputJson;
        std::vector<ResourceInfo> mResources;
    };
}