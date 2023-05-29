#pragma once
#include <string>
#include <sstream>

class Value {
public:
	typedef enum {
		NONE_T,
		NUM_T,
		BOOL_T,
		STRING_T,
	} datatype;

protected:
	datatype type;
	std::string StrRep;

public:
	Value();
	~Value();

	virtual void SetValue(float n);
	virtual void SetValue(bool b);
	
	void SetAsNone();

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


class StrValue : public Value {
protected:

public:
	StrValue(std::string& value);

	std::string GetValue();
	void SetValue(std::string& s);

	std::string operator+(StrValue next);
};