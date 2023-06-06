#pragma once

#include <iomanip>
#include <unordered_map>
#include <stack>

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

	RunnableValue* body;
	Chunk* CurrentChunk();
	void SetBody(RunnableValue* body);

	void RunCommand();

	Value* objects;

	static enum ExitCode {
		INTERPRET_OK = 0,
		UNRECOGNIZED_OPCODE = 201,
		STACK_UNDERFLOW,
		STACK_OVERFLOW,
		TYPE_ERROR,
		UNDEFINED_RAT,
		RETURN_FROM_SCRIPT,  // 'return' statement outside a runnable

		INTERNAL_ERROR,
	};


	BoolValue *NewObject(bool b);
	NumValue *NewObject(float f);
	StrValue* NewObject(std::string& s);
	Value* NewObject(nullptr_t p);

	bool GetBoolValue(Value *);
	float GetNumValue(Value *);
	std::string GetStrValue(Value*);

	std::string GetConstantStr(uint8_t index);
	float GetConstantNum(uint8_t index);
	bool GetConstantBool(uint8_t index);

	std::string TraceStack(int CodeOffset);
	void error(ExitCode e, std::string msg);

	std::unordered_map<std::string, Value*> globals;
	void AddGlobal(std::string&, Value*);
	Value* FindGlobal();

	Value* FindLocal();

public:
	Interpreter(RunnableValue *);
	~Interpreter();

	int interpret();
	Value* peek(int depth);

};

