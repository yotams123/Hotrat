#pragma once

#include <iomanip>

#include "Chunk.h"

#define DEBUG_TRACE_STACK

class Interpreter
{
private:
	static const short StackSize = 255;

	typedef struct {
		int stk[StackSize];
		uint8_t count;
	} StackStruct;

	StackStruct stack;
	void push(int value);
	int pop();

	Chunk* chunk;

	void RunCommand();

	static enum {
		INTERPRET_OK = 0,
		UNRECOGNIZED_OPCODE = 201,
		EMPTY_STACK,
		STACK_OVERFLOW,
	} ExitCode;

	std::string TraceStack(int CodeOffset);
	void error(int e, std::string msg);
public:
	Interpreter(Chunk*);
	~Interpreter();

	int interpret();

};

