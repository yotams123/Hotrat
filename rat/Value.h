#pragma once
#include <string>
#include <sstream>
#include <vector>

class ObjectValue;

class Value {
public:
	typedef enum datatype{
		NONE_T,
		NUM_T,
		BOOL_T,
		OBJECT_T,
	} datatype;

protected:
	datatype type;
	std::string StrRep;
	
	typedef union data {
		float n;
		bool b;
		ObjectValue* o;
	};

	data val;

public:
	Value();
	Value(float f);
	Value(bool b);
	Value(ObjectValue *o);

	~Value();

	void SetValue(float n);
	void SetValue(bool b);
	void SetValue(ObjectValue* o);
	void SetAsNone();

	float GetNum();
	bool GetBool();
	ObjectValue* GetObjectValue();
	bool IsNone();

	bool IsObject();

	datatype GetType();

	std::string& ToString();

	bool IsTruthy();
};

class ObjectValue {
public:
	typedef enum ObjectType{
		STRING_T,
		RUNNABLE_T,
		NATIVE_T
	} ObjectType;

protected:
	ObjectValue* next;
	std::string StrRep;

	ObjectType type;
	int references;

public:
	ObjectValue();

	ObjectType GetType();
	std::string& ToString();
	
	bool IsString();
	bool IsRunnable();
	bool IsNative();

	void SetNext(ObjectValue* obj);
	ObjectValue *GetNext();

	void AddReference();
	bool DeleteReference();
};

class StrValue : public ObjectValue {
public:
	StrValue(const std::string& value);

	std::string& GetValue();
	void SetValue(const std::string& s);

	std::string operator+(StrValue next);
};


struct Chunk;
class RunnableValue : public ObjectValue{
protected:
	struct Chunk *ByteCode;
	uint8_t arity;	// how many parameters the function accepts
	std::string name;

	RunnableValue* enclosing;
	
	std::vector<std::string> locals;
	uint8_t FrameStart; // index at which the frame starts

public:
	RunnableValue(struct Chunk *ByteCode); // for initializing the script
	RunnableValue(RunnableValue* enclosing, struct Chunk *ByteCode, std::vector<std::string>& args, const std::string& name);	// for use during compile time
	RunnableValue(RunnableValue* ToCopy, Chunk *chunk, RunnableValue *enclosing, uint8_t FrameStart);	// for use during the runtime
	~RunnableValue();

	Chunk* GetChunk();
	std::string& GetName();
	uint8_t GetArity();
	RunnableValue* GetEnclosing();
	uint8_t GetFrameStart();
	std::vector<std::string>& GetLocals();

	uint8_t AddLocal(std::string Identifier);
	short ResolveLocal(std::string Identifier);
};


class Interpreter;
typedef void (Interpreter::*NativeRunnable)();

class NativeValue : public ObjectValue {
private:
	std::string name;
	uint8_t arity;
	NativeRunnable runnable;

public:
	NativeValue(const std::string& name, uint8_t arity, NativeRunnable value);
	~NativeValue();

	NativeRunnable GetRunnable();
	uint8_t GetArity();
	std::string& GetName();
};