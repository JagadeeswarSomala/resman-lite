#include "ResHeaderParser.h"

#include <iostream>
#include <sstream>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

namespace resman
{

    ResHeaderParser& ResHeaderParser::setClangPath(const std::string& path)
    {
        mClangPath = path;
        return *this;
    }

    ResHeaderParser& ResHeaderParser::setHeaderFile(const std::string& path)
    {
        mHeaderFile = path;
        return *this;
    }

    ResHeaderParser& ResHeaderParser::setOutputJson(const std::string& path)
    {
        mOutputJson = path;
        return *this;
    }

    ResHeaderParser& ResHeaderParser::addIncludePath(const std::string& includeDir)
    {
        mIncludeDirs.push_back(includeDir);
        return *this;
    }

    ResHeaderParser& ResHeaderParser::addIncludePath(const std::vector<std::string>& includeDir)
    {
        mIncludeDirs.insert(mIncludeDirs.end(), includeDir.begin(), includeDir.end());
        return *this;
    }

    ResHeaderParser& ResHeaderParser::addDefine(const std::string& define)
    {
        mDefines.push_back(define);
        return *this;
    }

    ResHeaderParser& ResHeaderParser::addDefine(const std::vector<std::string>& define)
    {
        mDefines.insert(mDefines.end(), define.begin(), define.end());
        return *this;
    }

    bool ResHeaderParser::validateInputs() const
    {
        if (mHeaderFile.empty()) {
            std::cerr << "[ResHeaderParser] Error: header file not set.\n";
            return false;
        }

        if (!fs::exists(mHeaderFile)) {
            std::cerr << "[ResHeaderParser] Error: header file does not exist: " << mHeaderFile << "\n";
            return false;
        }

        if (mOutputJson.empty()) {
            std::cerr << "[ResHeaderParser] Error: output JSON path not set.\n";
            return false;
        }

        // clang path is optional; if not provided we will use "clang++" on PATH.
        return true;
    }

    std::string ResHeaderParser::getCommandLine() const
    {
        std::ostringstream cmd;
        // default clang binary
        std::string clangBin = mClangPath.empty() ? "clang++" : mClangPath;

        // Use flags to produce AST as JSON and only check syntax (-fsyntax-only).
        cmd << clangBin << " -Xclang -ast-dump=json -fsyntax-only -x c++-header ";

        // add include dirs
        for (const auto &inc : mIncludeDirs) {
            // Quote paths containing spaces
            if (inc.find(' ') != std::string::npos)
                cmd << "-I\"" << inc << "\" ";
            else
                cmd << "-I" << inc << " ";
        }

        // add defines
        for (const auto &d : mDefines) {
            if (d.find(' ') != std::string::npos)
                cmd << "-D\"" << d << "\" ";
            else
                cmd << "-D" << d << " ";
        }

        // input file (quote if needed)
        if (mHeaderFile.find(' ') != std::string::npos)
            cmd << "\"" << mHeaderFile << "\" ";
        else
            cmd << mHeaderFile << " ";

        // Redirect stdout to output JSON and stderr merged so diagnostics are visible
        // Using "2>&1" works on POSIX shells and Windows cmd.exe.
        if (mOutputJson.find(' ') != std::string::npos)
            cmd << "> \"" << mOutputJson << "\" 2>&1";
        else
            cmd << "> " << mOutputJson << " 2>&1";

        return cmd.str();
    }

    bool ResHeaderParser::invokeClangAST() const
    {
        std::string cmd = getCommandLine();

        std::cout << "[ResHeaderParser] Invoking: " << cmd << "\n";

        // Execute command
        int rc = std::system(cmd.c_str());

        if (rc != 0) {
            std::cerr << "[ResHeaderParser] clang returned non-zero exit code: " << rc << "\n";
            return false;
        }

        // Verify output file was created
        if (!fs::exists(mOutputJson)) {
            std::cerr << "[ResHeaderParser] Error: Expected output file not found: " << mOutputJson << "\n";
            return false;
        }

        return true;
    }

    bool ResHeaderParser::run()
    {
        if (!validateInputs())
            return false;

        // If clang path empty, rely on PATH; otherwise, if given, we could check accessibility,
        // but keep it simple and just attempt to run.
        bool ok = invokeClangAST();
        if (!ok) {
            std::cerr << "[ResHeaderParser] Error: AST generation failed.\n";
            return false;
        }

        return true;
    }

} // namespace resman
