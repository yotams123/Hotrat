#pragma once

#include <stack>

#include "Chunk.h"

class Interpreter
{
private:
	std::stack<int> temps;
	void push(int value);
	int pop();

	Chunk* chunk;

	void RunCommand();

	static enum {
		INTERPRET_OK,
		UNRECOGNIZED_OPCODE = 3,
		EMPTY_STACK,
		STACK_OVERFLOW,
	} ExitCode;
	int LineNum;

	void error(int e, std::string msg);
public:
	Interpreter(Chunk*);
	~Interpreter();

	int interpret();

};

