#include "Value.h"

Value::Value() {
	// 'none' value

	this->type = NONE_T; // temporary value, will be set by the actual type's initializer
	this->StrRep = "None";
}

Value::Value(float f) {
	// Number value
	this->val.n = f;
	this->type = NUM_T;

	std::stringstream s;
	s << f;
	this->StrRep = s.str();
}


Value::Value(bool b) {
	// Boolean value
	this->val.b = b;
	this->type = BOOL_T;

	if (b) this->StrRep = "true";
	else this->StrRep = "false";
}

Value::Value(ObjectValue* o) {
	// ObjectValue Value
	this->val.o = o;
	this->type = OBJECT_T;

	this->StrRep = o->ToString();
}

Value::~Value() {
}

Value::datatype Value::GetType() {
	return this->type;
}


void Value::SetValue(float n) {
	this->type = NUM_T;
	this->val.n = n;

	std::stringstream s;
	s << n;
	this->StrRep = s.str();
}

void Value::SetValue(bool b) {
	this->type = BOOL_T;
	this->val.b = b;

	if (b) this->StrRep = "true";
	else this->StrRep = "false";
}

void Value::SetValue(ObjectValue* o) {
	this->type = OBJECT_T;
	this->val.o = o;

	if (o != nullptr) {
		this->StrRep = o->ToString();
	}
	else {
		this->StrRep = "None";
	}
}

void Value::SetAsNone() {
	this->type = NONE_T;

	this->StrRep = "None";
}



float Value::GetNum() {
	return this->val.n;
}

bool Value::GetBool() {
	return this->val.b;
}

ObjectValue *Value::GetObjectValue() {
	return this->val.o;
}

bool Value::IsNone() {
	return this->type == NONE_T;
}

bool Value::IsObject() {
	return this->type == OBJECT_T;
}



std::string& Value::ToString() {	
	return this->StrRep;
}

bool Value::IsTruthy() {
	// If a value is truthy, it is equivalent to the value 'true' when read as a boolean.
	// Otherwise, it is falsey, meaning it is equivalent to the value 'false' when read as a boolean.
	switch (this->type)
	{
		case NUM_T:			return  this->GetNum() != 0;		break;
		case BOOL_T:		return	this->GetBool();			break;
		case OBJECT_T: {
			ObjectValue* o = this->val.o;
			switch (o->GetType())
			{
				case ObjectValue::STRING_T:		return o->ToString() != "" && o->ToString() != "false";		break;
				case ObjectValue::RUNNABLE_T:	return true;
				case ObjectValue::NATIVE_T:		return true;
				default:
					break;
			}
		}
											break;
		case NONE_T:		return false;									break;

		default:	return false;
	}
}

ObjectValue::ObjectValue() {
	this->references = 0;
	this->next = nullptr;
}

ObjectValue::ObjectType ObjectValue::GetType() {
	return this->type;
}

ObjectValue* ObjectValue::GetNext() {
	return this->next;
}

void ObjectValue::SetNext(ObjectValue* obj) {
	this->next = obj;
}

bool ObjectValue::IsString() {
	return this->type == STRING_T;
}

bool ObjectValue::IsRunnable() {
	return this->type == RUNNABLE_T;
}

bool ObjectValue::IsNative() {
	return this->type == NATIVE_T;
}

std::string& ObjectValue::ToString() {
	return this->StrRep;
}

void ObjectValue::AddReference() {
	this->references++;
}

bool ObjectValue::DeleteReference() {
	this->references--;
	if (this->references <= 0) {
		return true;
	}
	return false;
}


StrValue::StrValue(std::string& value) {
	this->type = STRING_T;
	this->StrRep = value;
}

std::string& StrValue::GetValue() {
	return this->StrRep;
}

void StrValue::SetValue(std::string& s) {
	this->StrRep = s;
}

std::string StrValue::operator+(StrValue next) {
	return this->StrRep + next.GetValue();
}


RunnableValue::RunnableValue(struct Chunk *ByteCode, uint8_t arity, std::string& name) {
	this->ByteCode = ByteCode;
	this->StrRep = name;
	this->arity = arity;

	this->type = RUNNABLE_T;
}

RunnableValue::RunnableValue(RunnableValue *enclosing, struct Chunk *ByteCode, std::vector<std::string>& args, std::string& name) {
	// Constructor for use during compile-time. Bytecode, args and name are all known at compile time.
	
	this->ByteCode = ByteCode;

	this->name = name;
	this->StrRep = "<Runnable '" + name + "'>";
	
	this->arity = args.size();

	this->enclosing = enclosing;

	for (int i = 0; i < args.size(); i++) this->locals.push_back(args[i]);
	
	this->FrameStart = 0; // temp value
	this->type = RUNNABLE_T;
}

RunnableValue::RunnableValue(RunnableValue* ToCopy, Chunk *chunk, RunnableValue *enclosing, uint8_t FrameStart) {
	// Constructor for use during runtime.
	// The Value returned is a copy of the original RunnableValue.
	// This allows for recursion
	// The start slot of the callframe is also known at runtime.

	this->ByteCode = chunk;

	this->name = ToCopy->name;
	this->StrRep = "<Runnable '" + name + "'>";
	
	this->arity = ToCopy->arity;
	this->type = RUNNABLE_T;
	this->locals = ToCopy->locals;

	this->FrameStart = FrameStart;

	this->enclosing = enclosing;
}


RunnableValue::~RunnableValue() {
	delete this->ByteCode;
	this->ByteCode = nullptr;
}

struct Chunk *RunnableValue::GetChunk() {
	return this->ByteCode;
}

std::string& RunnableValue::GetName() {
	return this->name;
}

uint8_t RunnableValue::GetArity() {
	return this->arity;
}

uint8_t RunnableValue::GetFrameStart() {
	return this->FrameStart;
}

std::vector<std::string>& RunnableValue::GetLocals() {
	return this->locals;
}

RunnableValue* RunnableValue::GetEnclosing() {
	return this->enclosing;
}

uint8_t RunnableValue::AddLocal(std::string Identifier) {
	// Add a new local variable

	this->locals.push_back(Identifier);
	return this->locals.size() - 1; // return the index of the last inserted item
}


short RunnableValue::ResolveLocal(std::string Identifier) {
	// Find the stack slot of a local variable
	
	for (int i = 0; i < this->locals.size(); i++) {
		if (this->locals[i] == Identifier) return i;
	}

	return -1;
}


NativeValue::NativeValue(std::string& name, uint8_t arity, NativeRunnable runnable) {
	this->type = NATIVE_T;
	this->arity = arity;
	this->runnable = runnable;

	this->StrRep = "<Native runnable '" + name + "'>";
}

NativeValue::~NativeValue() {
}

NativeRunnable NativeValue::GetRunnable() {
	return this->runnable;
}


uint8_t NativeValue::GetArity() {
	return this->arity;
}