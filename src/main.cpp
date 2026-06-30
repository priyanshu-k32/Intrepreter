#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include "builtins.hpp"
#include "list_utils.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

void runSource(const std::string& source, Interpreter& interp, EnvPtr env, bool printResults) {
    Lexer lexer(source);
    Parser parser(lexer.tokenize());
    auto exprs = parser.parseProgram();
    for (auto& e : exprs) {
        ValuePtr result = interp.eval(e, env);
        if (printResults && result->type != ValueType::Nil) {
            std::cout << toString(result) << "\n";
        }
    }
}

void repl(Interpreter& interp, EnvPtr env) {
    std::cout << "Mini-Lisp interpreter (C++). Type :quit to exit.\n";
    std::string line;
    while (true) {
        std::cout << "lisp> ";
        if (!std::getline(std::cin, line)) break;
        if (line == ":quit") break;
        if (line.empty()) continue;
        try {
            runSource(line, interp, env, true);
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }
}

int main(int argc, char** argv) {
    Interpreter interp;
    EnvPtr env = makeGlobalEnv();

    if (argc > 1) {
        std::ifstream file(argv[1]);
        if (!file) {
            std::cerr << "Could not open file: " << argv[1] << "\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        try {
            runSource(buffer.str(), interp, env, false);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    } else {
        repl(interp, env);
    }
    return 0;
}
