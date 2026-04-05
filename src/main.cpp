#include "interpreter.h"
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
    if (argc != 2) {
        std::cerr << "Usage: sturka <file.sturka>\n";
        return 1;
    }

    std::string path = argv[1];
    if (path.size() < 7 || path.substr(path.size() - 7) != ".sturka") {
        std::cerr << "Expected a .sturka source file\n";
        return 1;
    }

    try {
        Lexer lexer(readFile(path));
        Parser parser(lexer.tokenize());
        Program program = parser.parseProgram();

        Interpreter interpreter;
        interpreter.execute(program);
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "sturka error: " << error.what() << '\n';
        return 1;
    }
}
