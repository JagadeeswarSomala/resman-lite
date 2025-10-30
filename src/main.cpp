#include <iostream>
#include <filesystem>
#include <argparse/argparse.hpp>
#include "ResBuildOrchestrator.h"

int main(int argc, char** argv)
{
    argparse::ArgumentParser program("resman-lite", "0.1");

    program.add_description("Cross-platform resource-to-object generator using LLVM + Clang cli tools.");

    program.add_argument("-r", "--res-header")
        .help("Resource header file (Header file path containing resman::Resource<> declarations)")
        .required();

    program.add_argument("-o", "--obj-name")
        .help("Output object file name (e.g., resources.o or resources.obj)")
        .required();

    program.add_argument("-I", "--include-path")
        .help("Include paths (repeatable)")
        .append();

    program.add_argument("-R", "--res-path")
        .help("Resource directories (repeatable)")
        .append();

    program.add_argument("-t", "--mtriple")
        .help("Target triple (e.g., x86_64-pc-windows-msvc, x86_64-unknown-linux-gnu, aarch64-pc-windows-msvc etc. By default, the host triple is used)")
        .default_value(std::string(""));

    program.add_argument("-w", "--working-dir")
        .help("Working directory (optional; if not provided, a temporary one is used)")
        .default_value(std::string(""));

    program.add_argument("--clang-path")
        .help("Path to clang++ binary (optional, defaults to clang++ in PATH)")
        .default_value(std::string("clang++"));

    program.add_argument("--llvm-as-path")
        .help("Path to llvm-as binary (optional, defaults to llvm-as in PATH)")
        .default_value(std::string("llvm-as"));

    program.add_argument("--llvm-link-path")
        .help("Path to llvm-link binary (optional, defaults to llvm-link in PATH)")
        .default_value(std::string("llvm-link"));

    program.add_argument("--llc-path")
        .help("Path to llc binary (optional, defaults to llc in PATH)")
        .default_value(std::string("llc"));

    try
    {
        program.parse_args(argc, argv);

        if (program.get<bool>("--version"))
        {
            std::cout << "resman-lite version 0.1\n";
            return 0;
        }

        resman::BuildOptions opts;
        opts.resHeader = program.get<std::string>("--res-header");
        opts.outputObj = program.get<std::string>("--obj-name");

        if (program.is_used("--include-path"))
            opts.includePaths = program.get<std::vector<std::string>>("--include-path");

        if (program.is_used("--res-path"))
            opts.resPaths = program.get<std::vector<std::string>>("--res-path");

        opts.targetTriple = program.get<std::string>("--mtriple");

        std::string workingDir = program.get<std::string>("--working-dir");
        if (!workingDir.empty())
            opts.workingDir = workingDir;

        std::cout << "\nresman-lite configuration:\n";
        std::cout << "  Header        : " << opts.resHeader << "\n";
        std::cout << "  Output Object : " << opts.outputObj << "\n";
        if (!opts.includePaths.empty())
        {
            std::cout << "  Include Paths :\n";
            for (auto& p : opts.includePaths) std::cout << "    - " << p << "\n";
        }
        if (!opts.resPaths.empty())
        {
            std::cout << "  Resource Dirs :\n";
            for (auto& p : opts.resPaths) std::cout << "    - " << p << "\n";
        }
        if (!opts.targetTriple.empty())
            std::cout << "  Target Triple : " << opts.targetTriple << "\n";
        if (opts.workingDir.has_value())
            std::cout << "  Working Dir   : " << *opts.workingDir << "\n";
        std::cout << std::endl;

         // LLVM tool path arguments
        program.add_argument("--clang-path")
            .help("Path to clang++ binary (optional; defaults to clang++ in PATH)")
            .default_value(std::string("clang++"));

        program.add_argument("--llvm-as-path")
            .help("Path to llvm-as binary (optional; defaults to llvm-as in PATH)")
            .default_value(std::string("llvm-as"));

        program.add_argument("--llvm-link-path")
            .help("Path to llvm-link binary (optional; defaults to llvm-link in PATH)")
            .default_value(std::string("llvm-link"));

        program.add_argument("--llc-path")
            .help("Path to llc binary (optional; defaults to llc in PATH)")
            .default_value(std::string("llc"));

        resman::ResBuildOrchestrator orch;
        bool success = orch.setOptions(opts).run();

        if (success)
            std::cout << "Build completed successfully.\n";
        else
            std::cerr << "Build failed.\n";

        return success ? 0 : 1;
    }
    catch (const std::exception& err)
    {
        std::cerr << "Error: " << err.what() << "\n\n";
        std::cerr << program;
        return 1;
    }
}