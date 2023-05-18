#pragma once

#include <iomanip>

#include "Chunk.h"
#include "Value.h"

#define DEBUG_TRACE_STACK

class Interpreter
{
private:
	static const short StackSize = 255;

	typedef struct {
		Value *stk[StackSize];
		uint8_t count;
	} StackStruct;

	StackStruct stack;
	void push(Value *value);
	Value *pop();

	Chunk* chunk;

	void RunCommand();

	Value* objects;

	static enum {
		INTERPRET_OK = 0,
		UNRECOGNIZED_OPCODE = 201,
		STACK_UNDERFLOW,
		STACK_OVERFLOW,
		TYPE_ERROR
	} ExitCode;


	BoolValue *NewObject(bool b);
	NumValue *NewObject(float f);

	std::string TraceStack(int CodeOffset);
	void error(int e, std::string msg);

public:
	Interpreter(Chunk*);
	~Interpreter();

	int interpret();
	Value* peek(int depth);

};

