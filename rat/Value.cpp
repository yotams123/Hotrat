#include "Value.h"

Value::Value() {
	this->next = nullptr;
	this->IsNone = true;

	this->t = NUM_T; // temporary value, will be set by the actual type's initializer
	this->StrRep = "Error - value with no type";
}

Value::~Value() {

}


UniVal Value::GetValue() {
	UniVal v = { .n = {0} };
	switch (this->t)
	{
	case NUM_T:		v.n = ((NumValue*)this)->GetValue();	break;
	case BOOL_T:	v.b = ((BoolValue*)this)->GetValue();	break;
	case STRING_T:	v.s = this->StrRep.c_str();	break;
	default:
		break;
	}

	if (IsNone) {
		v.n = 0;
	}
	return v;
}

Value::datatype Value::GetType() {
	return this->t;
}


void Value::SetValue(float f) {
	((NumValue*)this)->SetValue(f);
}

void Value::SetValue(bool b) {
	((BoolValue*)this)->SetValue(b);
}

std::string Value::ToString() {	
	return this->StrRep;
}

void Value::SetAsNone() {
	IsNone = true;
}



NumValue::NumValue(float value) {
	this->value = value;
	this->t = NUM_T;
	this->IsNone = false;

	std::stringstream s;
	s << value;
	this->StrRep = s.str();
}

float NumValue::GetValue() {
	return this->value;
}

void NumValue::SetValue(float f) {
	this->value = f;
	this->IsNone = false;

	std::stringstream s;
	s << f;
	this->StrRep = s.str();
}


BoolValue::BoolValue(bool value) {
	this->value = value;
	this->t = BOOL_T;
	this->IsNone = false;

	if (value) this->StrRep = "true";
	else this->StrRep = "false";
}


bool BoolValue::GetValue() {
	return this->value;
}

void BoolValue::SetValue(bool b) {
	this->value = b;
	this->IsNone = false;

	if (b) this->StrRep = "true";
	else this->StrRep = "false";
}



StrValue::StrValue(std::string& value) {
	this->t = STRING_T;
	this->StrRep = value;
	
	this->IsNone = false;
}

std::string StrValue::GetValue() {
	return this->StrRep;
}

void StrValue::SetValue(std::string& s) {
	this->StrRep = s;

	this->IsNone = false;
}