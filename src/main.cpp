#include "ResHeaderParser.h"
#include "ResASTJsonParser.h"
#include <iostream>

int main()
{
    std::cout << "=== resman-lite: Test ===\n";

    // Step 1: Run clang++ AST export
    resman::ResHeaderParser parser;

    parser.setClangPath("clang++") // system clang++ in PATH
          .setHeaderFile("C:/Users/jsoma/OneDrive/Desktop/repo/resman-lite/test/resource_list.h")
          .setOutputJson("C:/Users/jsoma/OneDrive/Desktop/repo/resman-lite/test/resources_ast.json")
          .addIncludePath("C:/Users/jsoma/OneDrive/Desktop/repo/resman-lite/include")
          .addDefine("TEST_BUILD=1");

    std::cout << "Command line that will be run:\n"
              << parser.getCommandLine() << "\n\n";

    if (!parser.run()) {
        std::cerr << "❌ Failed to generate AST JSON from header.\n";
        return 1;
    }

    std::cout << "✅ Successfully generated AST JSON.\n\n";

    // Step 2: Parse the AST JSON
    resman::ResASTJsonParser astParser;
    astParser.setInputJson("C:/Users/jsoma/OneDrive/Desktop/repo/resman-lite/test/resources_ast.json")
             .setOutputJson("C:/Users/jsoma/OneDrive/Desktop/repo/resman-lite/test/resources_info.json");

    if (!astParser.run()) {
        std::cerr << "❌ Failed to parse AST JSON.\n";
        return 1;
    }

    std::cout << "✅ Successfully parsed resources from AST.\n";
    std::cout << "Output written to: resources_info.json\n\n";

    // Step 3: (Optional) Print summary
    const auto& resources = astParser.getResources();
    std::cout << "Extracted " << resources.size() << " resources:\n";
    for (const auto& r : resources) {
        std::cout << "  - VarName: " << r.resName
                  << "\n    Type: " << r.resType
                  << "\n    File: " << r.resFilepath << "\n\n";
    }

    return 0;
}
