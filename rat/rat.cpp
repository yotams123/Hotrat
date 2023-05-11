#include <iostream>
#include <fstream>
#include <string>

#define DEBUG_PRINT_CODE

#include "rat.h"

#include "Scanner.h"
#include "Token.h"
#include "Compiler.h"
#include "Interpreter.h"

#ifdef DEBUG_PRINT_CODE
#include "Debugger.h"
#endif


int main(int argc, char *argv[])
{
    if (argc > 2) {
        std::cout << "Usage: rats [file name]\n";
    }
    else if (argc == 2) {
        RunScript(argv[2]);
    }
    else {
        RunPrompt();
    }
    return 0;
}


void RunScript(std::string filename) {
    std::string buffer;
    std::ifstream Script(filename);
    int code = 0;

    if (!Script.is_open()) {
        std::cerr << "There was an error opening the file => quitting\n";
        return;
    }
    while (!Script.eof()) {
        Script >> buffer;
        code = Run(buffer);
        if (code == -1) return;
    }
}

void RunPrompt() {
    std::string buffer;

    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, buffer);
        Run(buffer);  // the function's return code doesn't matter when running a prompt
    }
}

int Run(std::string line) {
    Scanner scanner = Scanner(line);
    std::vector<Token>  tokens = scanner.ScanTokens();

    if (tokens.empty()) return -1; // found error while scanning

    Compiler compiler = Compiler(tokens);
    Chunk *script = compiler.Compile();

#ifdef DEBUG_PRINT_CODE
    Debugger debugger = Debugger(script, (std::string)"script");
    debugger.DisassembleChunk();
#endif // DEBUG_PRINT_CODE

    if (!script) return -1; // compilation error

    Interpreter interpreter = Interpreter(script);
    int code = interpreter.interpret();

    return code;
}
