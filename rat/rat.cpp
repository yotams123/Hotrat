#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>

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
        RunScript(argv[1]);
    }
    else {
        RunPrompt();
    }
    return 0;
}


void RunScript(char *filename) {
    std::string buffer;
    std::ifstream Script(filename);
    int code = 0;

    if (!Script.is_open()) {
        std::cerr << "There was an error opening the file => quitting\n";
        return;
    }
    
    std::ostringstream stream;
    stream << Script.rdbuf();
    buffer = stream.str();

    code = Run(buffer);
    if (code != 0) exit(code);
}

void RunPrompt() {
    std::string buffer;

    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, buffer);
        Run(buffer);  // the function's return code doesn't matter when running a prompt
    }
}

int Run(std::string& src) {
    Scanner *scanner = new Scanner(src);
    std::vector<Token>  tokens = scanner->ScanTokens();

    if (tokens.empty()) return 1; // scanner error
    delete scanner;

    Compiler *compiler = new Compiler(tokens);
    RunnableValue *script = compiler->Compile();

    if (!script) return 100; // compilation error
    delete compiler;

#ifdef DEBUG_PRINT_CODE
    Debugger *debugger = new Debugger(script->GetChunk(), (std::string)"script");
    
    std::thread DbgThread(&Debugger::DisassembleChunk, debugger);
#endif // DEBUG_PRINT_CODE


    Interpreter *interpreter = new Interpreter(script);
    int code = interpreter->interpret();
    delete interpreter;

#ifdef DEBUG_PRINT_CODE
    DbgThread.join();
    delete debugger;
#endif // DEBUG_PRINT_CODE

    return code;
}
