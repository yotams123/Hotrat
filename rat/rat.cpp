#include <iostream>
#include <fstream>
#include <string>

#include "rat.h"
#include "scanner.h"
#include "Token.h"

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

    for (Token token : tokens) { 
        std::cout << token.ToString() << "\n";
    }

    return 0;
}
