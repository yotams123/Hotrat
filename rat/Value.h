#pragma once
#include <string>
#include <sstream>

typedef union {
	float n;
	bool b;
} UniVal;

class Value {
public:
	typedef enum {
		NUM_T,
		BOOL_T
	} datatype;

protected:
	datatype t;
	std::string StrRep;

public:
	Value();
	~Value();

	UniVal GetValue();
	virtual void SetValue(float n);
	virtual void SetValue(bool b);

	Value* next;
	datatype GetType();

	std::string ToString();
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