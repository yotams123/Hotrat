#pragma once

#include "Scanner.h"
#include "Token.h"
#include "Compiler.h"
#include "Interpreter.h"

#ifdef DEBUG_PRINT_CODE 
#include "Debugger.h"
#endif

void RunScript(char *filename);
void RunPrompt();
int Run(std::string& line);