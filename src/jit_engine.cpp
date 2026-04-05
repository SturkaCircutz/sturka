#include "jit_engine.h"

#include <Windows.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
class JitSourceBuilder {
public:
    std::string build(const Program& program) {
        line("#include <cstddef>");
        line("#include <cstdint>");
        line("#include <cstring>");
        line("#include <iostream>");
        line("#include <stdexcept>");
        line("#include <string>");
        line("#include <unordered_map>");
        line("");
        line("using Variables = std::unordered_map<std::string, std::int64_t>;");
        line("");
        line("static void sturka_set_error(char* buffer, std::size_t bufferSize, const std::string& message) {");
        indent();
        line("if (buffer == nullptr || bufferSize == 0) {");
        indent();
        line("return;");
        dedent();
        line("}");
        line("const std::size_t copyLength = message.size() < (bufferSize - 1) ? message.size() : (bufferSize - 1);");
        line("std::memcpy(buffer, message.data(), copyLength);");
        line("buffer[copyLength] = '\\0';");
        dedent();
        line("}");
        line("");
        line("static void sturka_let(Variables& vars, const char* name, std::int64_t value) {");
        indent();
        line("const auto [it, inserted] = vars.emplace(name, value);");
        line("if (!inserted) {");
        indent();
        line("throw std::runtime_error(std::string(\"Variable already defined: \") + name);");
        dedent();
        line("}");
        dedent();
        line("}");
        line("");
        line("static void sturka_assign(Variables& vars, const char* name, std::int64_t value) {");
        indent();
        line("auto it = vars.find(name);");
        line("if (it == vars.end()) {");
        indent();
        line("throw std::runtime_error(std::string(\"Undefined variable: \") + name);");
        dedent();
        line("}");
        line("it->second = value;");
        dedent();
        line("}");
        line("");
        line("static std::int64_t sturka_get(Variables& vars, const char* name) {");
        indent();
        line("auto it = vars.find(name);");
        line("if (it == vars.end()) {");
        indent();
        line("throw std::runtime_error(std::string(\"Undefined variable: \") + name);");
        dedent();
        line("}");
        line("return it->second;");
        dedent();
        line("}");
        line("");
        line("static std::int64_t sturka_div(std::int64_t left, std::int64_t right) {");
        indent();
        line("if (right == 0) {");
        indent();
        line("throw std::runtime_error(\"Division by zero\");");
        dedent();
        line("}");
        line("return left / right;");
        dedent();
        line("}");
        line("");
        line("extern \"C\" __declspec(dllexport) int sturka_run(char* errorBuffer, std::size_t errorBufferSize) {");
        indent();
        line("try {");
        indent();
        line("Variables vars;");
        for (const auto& statement : program) {
            emitStatement(*statement);
        }
        line("return 0;");
        dedent();
        line("} catch (const std::exception& ex) {");
        indent();
        line("sturka_set_error(errorBuffer, errorBufferSize, ex.what());");
        line("return 1;");
        dedent();
        line("}");
        dedent();
        line("}");
        return output_.str();
    }

private:
    void emitStatement(const Stmt& stmt) {
        switch (stmt.kind) {
        case StmtKind::Let: {
            const auto& letStmt = static_cast<const LetStmt&>(stmt);
            line("sturka_let(vars, \"" + letStmt.name + "\", " + emitExpr(*letStmt.value) + ");");
            break;
        }
        case StmtKind::Assign: {
            const auto& assignStmt = static_cast<const AssignStmt&>(stmt);
            line("sturka_assign(vars, \"" + assignStmt.name + "\", " + emitExpr(*assignStmt.value) + ");");
            break;
        }
        case StmtKind::Print: {
            const auto& printStmt = static_cast<const PrintStmt&>(stmt);
            line("std::cout << " + emitExpr(*printStmt.value) + " << '\\n';");
            break;
        }
        case StmtKind::Block:
            emitBlock(static_cast<const BlockStmt&>(stmt));
            break;
        case StmtKind::If: {
            const auto& ifStmt = static_cast<const IfStmt&>(stmt);
            line("if (" + emitExpr(*ifStmt.condition) + " != 0) {");
            indent();
            emitBlock(*ifStmt.thenBranch);
            dedent();
            if (ifStmt.elseBranch) {
                line("} else {");
                indent();
                emitBlock(*ifStmt.elseBranch);
                dedent();
            }
            line("}");
            break;
        }
        case StmtKind::While: {
            const auto& whileStmt = static_cast<const WhileStmt&>(stmt);
            line("while (" + emitExpr(*whileStmt.condition) + " != 0) {");
            indent();
            emitBlock(*whileStmt.body);
            dedent();
            line("}");
            break;
        }
        }
    }

    void emitBlock(const BlockStmt& block) {
        for (const auto& statement : block.statements) {
            emitStatement(*statement);
        }
    }

    std::string emitExpr(const Expr& expr) {
        switch (expr.kind) {
        case ExprKind::Integer:
            return std::to_string(static_cast<const IntegerExpr&>(expr).value) + "LL";
        case ExprKind::Variable:
            return "sturka_get(vars, \"" + static_cast<const VariableExpr&>(expr).name + "\")";
        case ExprKind::Unary: {
            const auto& unaryExpr = static_cast<const UnaryExpr&>(expr);
            return "(" + unaryExpr.op + "(" + emitExpr(*unaryExpr.operand) + "))";
        }
        case ExprKind::Binary: {
            const auto& binaryExpr = static_cast<const BinaryExpr&>(expr);
            const std::string left = emitExpr(*binaryExpr.left);
            const std::string right = emitExpr(*binaryExpr.right);
            if (binaryExpr.op == "/") {
                return "sturka_div(" + left + ", " + right + ")";
            }
            return "(" + left + " " + binaryExpr.op + " " + right + ")";
        }
        }

        throw std::runtime_error("Unknown expression kind during JIT generation");
    }

    void line(const std::string& text) {
        for (int i = 0; i < indentLevel_; ++i) {
            output_ << "    ";
        }
        output_ << text << '\n';
    }

    void indent() {
        ++indentLevel_;
    }

    void dedent() {
        --indentLevel_;
    }

    std::ostringstream output_;
    int indentLevel_ = 0;
};

using JitEntry = int (*)(char*, std::size_t);

std::string readFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Could not read file: " + path.string());
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

struct TempArtifacts {
    std::filesystem::path directory;
    std::filesystem::path sourcePath;
    std::filesystem::path dllPath;
    std::filesystem::path logPath;
};

TempArtifacts makeTempArtifacts() {
    const auto uniqueValue =
        std::to_string(static_cast<long long>(std::chrono::steady_clock::now().time_since_epoch().count()));
    TempArtifacts artifacts;
    artifacts.directory = std::filesystem::temp_directory_path() / ("sturka_jit_" + uniqueValue);
    std::filesystem::create_directories(artifacts.directory);
    artifacts.sourcePath = artifacts.directory / "jit_module.cpp";
    artifacts.dllPath = artifacts.directory / "jit_module.dll";
    artifacts.logPath = artifacts.directory / "build.log";
    return artifacts;
}

void removeArtifacts(const TempArtifacts& artifacts) {
    std::error_code ignored;
    std::filesystem::remove_all(artifacts.directory, ignored);
}
}

void JitEngine::execute(const Program& program) {
    const TempArtifacts artifacts = makeTempArtifacts();
    try {
        JitSourceBuilder builder;
        {
            std::ofstream source(artifacts.sourcePath, std::ios::binary);
            if (!source) {
                throw std::runtime_error("Could not write JIT source file");
            }
            source << builder.build(program);
        }

        const std::string command =
            "g++ -std=c++17 -shared -static-libgcc -static-libstdc++ -o \"" +
            artifacts.dllPath.string() + "\" \"" + artifacts.sourcePath.string() +
            "\" > \"" + artifacts.logPath.string() + "\" 2>&1";
        const int exitCode = std::system(command.c_str());
        if (exitCode != 0) {
            throw std::runtime_error("JIT compilation failed:\n" + readFile(artifacts.logPath));
        }

        HMODULE module = LoadLibraryA(artifacts.dllPath.string().c_str());
        if (module == nullptr) {
            throw std::runtime_error("Could not load generated JIT module");
        }

        const auto cleanupModule = [&]() {
            FreeLibrary(module);
        };

        auto* rawFunction = GetProcAddress(module, "sturka_run");
        JitEntry function = nullptr;
        static_assert(sizeof(function) == sizeof(rawFunction), "Function pointer size mismatch");
        std::memcpy(&function, &rawFunction, sizeof(function));
        if (function == nullptr) {
            cleanupModule();
            throw std::runtime_error("Generated JIT module is missing sturka_run");
        }

        char errorBuffer[2048] = {};
        const int runResult = function(errorBuffer, sizeof(errorBuffer));
        cleanupModule();

        if (runResult != 0) {
            throw std::runtime_error(errorBuffer[0] != '\0' ? errorBuffer : "Unknown JIT runtime error");
        }

        removeArtifacts(artifacts);
    } catch (...) {
        removeArtifacts(artifacts);
        throw;
    }
}
