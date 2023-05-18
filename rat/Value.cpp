#include "Value.h"

Value::Value() {
	this->next = nullptr;
	this->t = NUM_T; // temporary value, will be set by the actual type's initializer
	this->StrRep = "Error - value with no type";
}

Value::~Value() {

}


UniVal Value::GetValue() {
	UniVal v; v.n = 0;
	switch (this->t)
	{
	case NUM_T:		v.n = ((NumValue*)this)->GetValue();	break;
	case BOOL_T:	v.b = ((BoolValue*)this)->GetValue();	break;
	default:
		break;
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


NumValue::NumValue(float value) {
	this->value = value;
	this->t = NUM_T;
	
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
	this->t = BOOL_T;
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