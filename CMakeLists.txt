cmake_minimum_required(VERSION 3.19)

project("resman-lite")

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/ResHeaderParser.cpp
    src/ResASTJsonParser.cpp
    src/ResCppSrcGenerator.cpp
    src/ResObjGenerator.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/submodules/json/include
)