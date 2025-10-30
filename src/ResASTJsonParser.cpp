#include "ResASTJsonParser.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace resman
{
    ResASTJsonParser& ResASTJsonParser::setInputJson(const std::string& path)
    {
        mInputJson = path;
        return *this;
    }

    ResASTJsonParser& ResASTJsonParser::setOutputJson(const std::string& path)
    {
        mOutputJson = path;
        return *this;
    }

    bool ResASTJsonParser::validateInputs() const
    {
        if (mInputJson.empty()) {
            std::cerr << "[ResASTJsonParser] Error: input JSON path not set.\n";
            return false;
        }

        if (!fs::exists(mInputJson)) {
            std::cerr << "[ResASTJsonParser] Error: input JSON does not exist: " << mInputJson << "\n";
            return false;
        }

        return true;
    }

    bool ResASTJsonParser::parseASTJson()
    {
        std::ifstream in(mInputJson);
        if (!in) {
            std::cerr << "[ResASTJsonParser] Error: failed to open " << mInputJson << "\n";
            return false;
        }

        json ast;
        try {
            in >> ast;
        }
        catch (const std::exception& e) {
            std::cerr << "[ResASTJsonParser] JSON parse error: " << e.what() << "\n";
            return false;
        }

        extractResources(ast);
        return true;
    }

    void ResASTJsonParser::extractResources(const json& node)
    {
        if (!node.is_object())
            return;

        std::string kind = node.value("kind", "");

        // Detect `VarDecl` for `resman::Resource<N>`
        if (kind == "VarDecl")
        {
            const std::string type = node["type"].value("qualType", "");

            if (type.find("resman::Resource<") != std::string::npos)
            {
                ResourceInfo info;

                // Full type string (e.g. "const resman::Resource<1>")
                info.resType = type;

                // Variable name
                info.resName = node.value("name", "");

                // Traverse to find string literal file path
                if (node.contains("inner") && node["inner"].is_array())
                {
                    for (const auto& child : node["inner"])
                    {
                        if (child.value("kind", "") == "CXXConstructExpr" && child.contains("inner"))
                        {
                            for (const auto& grandchild : child["inner"])
                            {
                                if (grandchild.value("kind", "") == "StringLiteral")
                                {
                                    std::string rawValue = grandchild.value("value", "");
                                    // Remove wrapping quotes if present
                                    if (rawValue.size() >= 2 && rawValue.front() == '"' && rawValue.back() == '"')
                                        rawValue = rawValue.substr(1, rawValue.size() - 2);

                                    info.resFilepath = rawValue;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (!info.resFilepath.empty())
                {
                    mResources.push_back(info);
                }
            }
        }

        // Recurse into nested children
        if (node.contains("inner") && node["inner"].is_array())
        {
            for (const auto& child : node["inner"])
            {
                extractResources(child);
            }
        }
    }

    bool ResASTJsonParser::run()
    {
        if (!validateInputs())
            return false;

        if (!parseASTJson())
            return false;

        if (!mOutputJson.empty()) {
            std::ofstream out(mOutputJson);
            if (out) {
                json outData = json::array();
                for (const auto& r : mResources) {
                    outData.push_back({
                        {"type", r.resType},
                        {"name", r.resName},
                        {"path", r.resFilepath}
                    });
                }
                out << outData.dump(4);
                std::cout << "[ResASTJsonParser] Extracted info written to " << mOutputJson << "\n";
            }
        }

        printSummary();
        return true;
    }

    void ResASTJsonParser::printSummary() const
    {
        std::cout << "\n=== Resource Summary ===\n";
        for (const auto& r : mResources) {
            std::cout << "Res: " << r.resType
                    << ", Var: " << r.resName
                    << ", Path: " << r.resFilepath << "\n";
        }
    }

} // namespace resman
