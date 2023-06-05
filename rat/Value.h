#pragma once
#include <string>
#include <sstream>
#include <map>

class Value {
public:
	typedef enum {
		NONE_T,
		NUM_T,
		BOOL_T,
		STRING_T,
		RUNNABLE_T,
	} datatype;

protected:
	datatype type;
	std::string StrRep;

public:
	Value();
	virtual ~Value();

	virtual void SetValue(float n);
	virtual void SetValue(bool b);
	
	void SetAsNone();

	Value* next;
	datatype GetType();

	std::string& ToString();

	bool IsTruthy();
};

class NumValue : public Value {
protected:
	float value;

public:
	NumValue(float value);

	float GetValue();
	void SetValue(float f);

};


class BoolValue : public Value {
protected:
	bool value;

public:
	BoolValue(bool value);

	bool GetValue();
	void SetValue(bool b);
};


class StrValue : public Value {
public:
	StrValue(std::string& value);

	std::string GetValue();
	void SetValue(std::string& s);

	std::string operator+(StrValue next);
};


struct Chunk;
class RunnableValue : public Value {
protected:
	struct Chunk *ByteCode;
	uint8_t arity;	// how many parameters the function accepts
	std::string name;

	RunnableValue* enclosing;
	std::map<std::string, nullptr_t> locals;

public:
	RunnableValue(struct Chunk *ByteCode, uint8_t arity, std::string name);
	RunnableValue(RunnableValue* enclosing, struct Chunk *ByteCode, uint8_t arity, std::string name);
	RunnableValue(RunnableValue* ToCopy, RunnableValue *enclosing);		// when calling in the runtime, call a copy of the value, to allow recursion (ip)
	~RunnableValue() override;

	Chunk* GetChunk();
	uint8_t GetArity();
	RunnableValue* GetEnclosing();
};