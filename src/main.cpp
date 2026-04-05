#include "interpreter.h"
#include "jit_engine.h"
#include "lexer.h"
#include "parser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not open file: " + path);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
}

int main(int argc, char** argv) {
    bool useJit = false;
    std::string path;

    if (argc == 2) {
        path = argv[1];
    } else if (argc == 3 && std::string(argv[1]) == "--jit") {
        useJit = true;
        path = argv[2];
    } else {
        std::cerr << "Usage: sturka [--jit] <file.sturka>\n";
        return 1;
    }

    if (path.size() < 7 || path.substr(path.size() - 7) != ".sturka") {
        std::cerr << "Expected a .sturka source file\n";
        return 1;
    }

    try {
        Lexer lexer(readFile(path));
        Parser parser(lexer.tokenize());
        Program program = parser.parseProgram();

        if (useJit) {
            JitEngine jit;
            jit.execute(program);
        } else {
            Interpreter interpreter;
            interpreter.execute(program);
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "sturka error: " << error.what() << '\n';
        return 1;
    }
}
