#include "Value.h"

Value::Value() {
	this->next = nullptr;

	this->type = NONE_T; // temporary value, will be set by the actual type's initializer
	this->StrRep = "None";
}

Value::~Value() {
}

Value::datatype Value::GetType() {
	return this->type;
}


void Value::SetValue(float f) {
	((NumValue*)this)->SetValue(f);
}

void Value::SetValue(bool b) {
	((BoolValue*)this)->SetValue(b);
}

std::string& Value::ToString() {	
	return this->StrRep;
}

bool Value::IsTruthy() {
	switch (this->type)
	{
		case NUM_T:			return ((NumValue*)this)->GetValue() != 0;		break;
		case BOOL_T:		return ((BoolValue*)this)->GetValue();			break;
		case STRING_T:		return ((StrValue*)this)->GetValue() != "";		break;
		case RUNNABLE_T:	return true;									break;
		case NONE_T:		return false;									break;

		default:	return false;
	}
}


NumValue::NumValue(float value) {
	this->value = value;
	this->type = NUM_T;
	
	std::stringstream s;
	s << value;
	this->StrRep = s.str();
}

float NumValue::GetValue() {
	return this->value;
}

void NumValue::SetValue(float f) {
	this->value = f;

	std::stringstream s;
	s << f;
	this->StrRep = s.str();
}


BoolValue::BoolValue(bool value) {
	this->value = value;
	this->type = BOOL_T;

	if (value) this->StrRep = "true";
	else this->StrRep = "false";
}


bool BoolValue::GetValue() {
	return this->value;
}

void BoolValue::SetValue(bool b) {
	this->value = b;

	if (b) this->StrRep = "true";
	else this->StrRep = "false";
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

RunnableValue::RunnableValue(RunnableValue *enclosing, struct Chunk *ByteCode, uint8_t arity, std::string name) {
	this->ByteCode = ByteCode;
	this->StrRep = name;
	this->arity = arity;

	this->enclosing = enclosing;

	this->type = RUNNABLE_T;
}

RunnableValue::RunnableValue(RunnableValue* ToCopy, RunnableValue *enclosing) {
	this->ByteCode = ToCopy->ByteCode;
	this->StrRep = ToCopy->StrRep;
	this->arity = ToCopy->arity;
	this->type = RUNNABLE_T;

	this->enclosing = enclosing;
}


RunnableValue::~RunnableValue() {
	delete this->ByteCode;
	this->ByteCode = nullptr;
}

struct Chunk *RunnableValue::GetChunk() {
	return this->ByteCode;
}

uint8_t RunnableValue::GetArity() {
	return this->arity;
}

RunnableValue* RunnableValue::GetEnclosing() {
	return this->enclosing;
}