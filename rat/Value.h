#pragma once
#include <string>
#include <sstream>

typedef union {
	float n;
	bool b;
	const char *s;
} UniVal;

class Value {
public:
	typedef enum {
		NUM_T,
		BOOL_T,
		STRING_T,
	} datatype;

protected:
	datatype t;
	std::string StrRep;

	bool IsNone;

public:
	Value();
	~Value();

	UniVal GetValue();
	virtual void SetValue(float n);
	virtual void SetValue(bool b);

	Value* next;
	datatype GetType();

	void SetAsNone();

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


class StrValue : public Value {
protected:

public:
	StrValue(std::string& value);

	std::string GetValue();
	void SetValue(std::string& s);
};