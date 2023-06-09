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
		Value stk[StackSize];
		uint8_t count;
	} StackStruct;

	StackStruct stack;

	void push(Value value);
	Value pop();

	RunnableValue* body;
	Chunk* CurrentChunk();
	void SetBody(RunnableValue* body);

	void RunCommand();

	ObjectValue* objects;

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

	std::string GetConstantStr(uint8_t index);
	float GetConstantNum(uint8_t index);
	bool GetConstantBool(uint8_t index);

	std::string TraceStack(int CodeOffset);
	void error(ExitCode e, std::string msg);

	std::unordered_map<std::string, Value> globals;
	void AddGlobal(std::string&, Value);
	bool IsDefinedGlobal(std::string& identifier);
	
	Value *FindGlobal();
	Value *FindLocal();

	Value NewObject(ObjectValue* obj);
	Value NewObject(std::string& str);

	StrValue* ExtractStrValue(Value* v, std::string&);
public:
	Interpreter(RunnableValue *);
	~Interpreter();

	int interpret();
	Value& peek(int depth);

};