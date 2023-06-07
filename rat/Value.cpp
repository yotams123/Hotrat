#include "Value.h"

Value::Value() {
	this->next = nullptr;

	this->type = NONE_T; // temporary value, will be set by the actual type's initializer
	this->StrRep = "None";
}

Value::Value(float f) {
	this->val.n = f;
	this->type = NUM_T;

	std::stringstream s;
	s << f;
	this->StrRep = s.str();
}


Value::Value(bool b) {
	this->val.b = b;
	this->type = BOOL_T;

	if (b) this->StrRep = "true";
	else this->StrRep = "false";
}

Value::Value(ObjectValue* o) {
	this->val.o = o;
	this->type = OBJECT_T;

	this->StrRep = o->ToString();
}

Value::~Value() {
}

Value::datatype Value::GetType() {
	return this->type;
}


void Value::SetValue(float f) {
	this->type = NUM_T;
	this->val.n = f;

	std::stringstream s;
	s << f;
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

	this->StrRep = o->ToString();


}

void Value::SetAsNone() {
	this->type = NONE_T;
}



float Value::GetNum() {
	return this->val.n;
}

bool Value::GetBool() {
	return this->val.b;
}

ObjectValue *Value::GetObject() {
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
	switch (this->type)
	{
		case NUM_T:			return  this->GetNum() != 0;		break;
		case BOOL_T:		return	this->GetBool();			break;
		case OBJECT_T: {
			ObjectValue* o = this->val.o;
			switch (o->GetType())
			{
				case ObjectValue::STRING_T:		return ((StrValue*)o)->GetValue() != "";		break;
				case ObjectValue::RUNNABLE_T:	return true;
				default:
					break;
			}
		}
											break;
		case NONE_T:		return false;									break;

		default:	return false;
	}
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


std::string ObjectValue::ToString() {
	return this->StrRep;
}



StrValue::StrValue(std::string& value) {
	this->type = STRING_T;
	this->StrRep = value;
	
}

std::string StrValue::GetValue() {
	return this->StrRep;
}

void StrValue::SetValue(std::string& s) {
	this->StrRep = s;
}

std::string StrValue::operator+(StrValue next) {
	return this->StrRep + next.GetValue();
}


RunnableValue::RunnableValue(struct Chunk *ByteCode, uint8_t arity, std::string name) {
	this->ByteCode = ByteCode;
	this->StrRep = name;
	this->arity = arity;

	this->type = RUNNABLE_T;
}

RunnableValue::RunnableValue(RunnableValue *enclosing, struct Chunk *ByteCode, std::vector<std::string>& args, std::string name) {
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
	this->locals.push_back(Identifier);
	return this->locals.size() - 1; // return the index of the last inserted item
}


short RunnableValue::ResolveLocal(std::string Identifier) {
	for (int i = 0; i < this->locals.size(); i++) {
		if (this->locals[i] == Identifier) return i;
	}

	return -1;
}