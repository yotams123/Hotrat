#pragma once

#include <iomanip>
#include <unordered_map>

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
	StrValue* NewObject(std::string& s);
	Value* NewObject(nullptr_t p);

	std::string GetConstantStr(uint8_t index);
	float GetConstantNum(uint8_t index);
	bool GetConstantBool(uint8_t index);

	std::string TraceStack(int CodeOffset);
	void error(int e, std::string msg);

	std::unordered_map<std::string, Value*> globals;
	void AddGlobal(std::string&, Value*);

public:
	Interpreter(Chunk*);
	~Interpreter();

	int interpret();
	Value* peek(int depth);

};

