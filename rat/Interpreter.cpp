#include "Interpreter.h"

#include <Windows.h>

Value NewValue(float f) {
	return Value(f);
}


Value NewValue(bool b) {
	return Value(b);
}

Value Interpreter::NewObject(std::string& s) {
	StrValue* res = new StrValue(s);
	Value v = this->NewObject(res);

	return v;
}

Value Interpreter::NewObject(ObjectValue* o) {
	// Create a new object and at it to the linked list

	Value v = Value(o);
	ObjectValue* obj = v.GetObjectValue();

	obj->SetNext(this->objects);
	this->objects = obj;
	return v;
}


Value NewValue() {
	// 'none' value
	return Value();
}


float GetNumValue(Value& v) {
	return v.GetNum();
}

bool GetBoolValue(Value& v) {
	return v.GetBool();
}

ObjectValue* GetObjectValue(Value& v) {
	return v.GetObjectValue();
}

bool ValueIsNone(Value& v) {
	return v.GetType() == Value::NONE_T;
}

bool IsIntegerValue(Value& f) {
	if (f.GetType() != Value::NUM_T) return false;
	float n = GetNumValue(f);
	if (n == (int)n) return true;
	return false;
}




void Interpreter::NativeInput() {
	// Code for native runnable, to get input as string

	Value PreInput = peek(0);
	std::string errormsg = "Argument to 'input' must be a string";
	StrValue * s = ExtractStrValue(&PreInput, errormsg);
	
	std::cout << s->GetValue();
	std::string str;
	std::getline(std::cin, str);

	pop();  // remove reference to 'PreInput'
	Value t = NewObject(str);
	push(t);
}

void Interpreter::NativePrint() {
	// Code for native runnable, to print a value to the screen.

	Value v = peek(0);
	std::cout << v.ToString() + "\n";
	pop();  // remove reference to v

	Value t = NewValue();
	push(t);  // none
}

void Interpreter::NativeReadFromFile() {
	// Code for native runnable that reads an entire file and returns a strvalue

	Value v = peek(0);  // Keep value in stack so it still has at least one reference

	std::string msg = "Argument to 'ReadFromFile' must be a valid path string";
	std::string FileName = ExtractStrValue(&v, msg)->ToString();
	LPCSTR filename = FileName.c_str();

	HANDLE handle = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) error(INTERNAL_ERROR, "There was an error opening the file '" + FileName + 
		"'.\n\tAre you sure the path is correct ? ");
	
	DWORD NumToReadH;
	DWORD NumToReadL = GetFileSize(handle, &NumToReadH);
	
	const int toread = ((NumToReadH << 8) | (NumToReadL));
	char* buff = new char[toread + 1];

	bool success = ReadFile(handle, buff, toread, NULL, NULL);
	if (success == FALSE) error(INTERNAL_ERROR, "Error reading the file '" + FileName + "'");
	buff[toread] = 0;

	std::string strbuf = std::string(buff);

	delete[] buff;
	buff = nullptr;

	CloseHandle(handle);
	
	pop();	// remove reference to filename string

	Value t = NewObject(strbuf);
	push(t);
}

void Interpreter::NativeWriteToFile() {
	// Code for native runnable that appends new text to end of file

	Value BuffValue = peek(0);  // Keep value in stack so it still has at least one reference
	std::string errormsg = "Arguments to 'WriteToFile' must be strings";

	std::string buff = ExtractStrValue(&BuffValue, errormsg)->ToString();  // Value to insert

	Value FileValue = peek(1);
	std::string filename = ExtractStrValue(&FileValue, errormsg)->ToString(); // File to insert to
	LPCSTR CFileName = filename.c_str();

	DWORD attribute = GetFileAttributesA(CFileName);
	if (attribute != INVALID_FILE_ATTRIBUTES) {
		bool ReadOnly = attribute & FILE_ATTRIBUTE_READONLY;
		if (ReadOnly) error(INTERNAL_ERROR, "The file '" + filename +
			"' cannot be written to.\n\tAre you sure the path is correct ? ");
	}

	HANDLE handle = CreateFileA(CFileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		error(INTERNAL_ERROR, "There was an error opening the file '" + filename +
			"'.\n\tAre you sure the path is correct ? ");
	}

	SetFilePointer(handle, 0, NULL, FILE_END);

	bool success = WriteFile(handle, buff.c_str(), buff.size(), NULL, NULL);
	if (!success) {
		error(INTERNAL_ERROR, "Error writing to file '" + filename + "'");
	}

	CloseHandle(handle);

	pop(); // remove reference to BuffValue
	pop(); // remove reference to FileValue

	Value t = NewValue();
	push(t);  // none
}

void Interpreter::NativeEmptyFile() {
	// Code for native runnable that clears the contents of a file

	Value FileValue = peek(0);  // Keep value in stack so it still has at least one reference

	std::string msg = "EmptyFile's argument must be a string value";

	StrValue * filestr = ExtractStrValue(&FileValue, msg);
	std::string filename = filestr->GetValue();

	DWORD attribute = GetFileAttributesA(filename.c_str());
	if (attribute != INVALID_FILE_ATTRIBUTES) {
		bool ReadOnly = attribute & FILE_ATTRIBUTE_READONLY;
		if (ReadOnly) error(INTERNAL_ERROR, "The file '" + filename +
			"' cannot be emptied.\n\tAre you sure the path is correct ? ");
	}

	HANDLE handle = CreateFileA(filename.c_str(), GENERIC_WRITE, NULL, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == 0) error(INTERNAL_ERROR, "There was an error emptying the file '" + filename +
		"'.\n\tAre you sure the path is correct ? ");

	CloseHandle(handle);

	pop(); // remove reference to FileValue

	Value v = NewValue();
	push(v); // none
}

void Interpreter::NativeConvertToNum() {
	// Code for a native function that converts a value to a Number

	Value v = peek(0);  // Keep value in stack so it still has at least one reference
	float num;
	switch (v.GetType())
	{
		case Value::BOOL_T:  num = (v.GetBool() ? 1 : 0); break;
		case Value::NUM_T:	 push(v);	return;
		case Value::NONE_T:	 num = 0;   break;

		default: {
			std::string errormsg = "Value given to 'Number()' must be of valid type";
			StrValue* s = ExtractStrValue(&v, errormsg);

			try {
				num = std::stof(s->ToString());
			}
			catch (std::invalid_argument e) {
				error(TYPE_ERROR, "Argument to " + globals["Number"].ToString() + " must be representable as a number");
			}
			break;
		}
	}

	pop(); // remove reference to v

	Value t = NewValue(num);
	push(t);
}

void Interpreter::NativeConvertToBool() {
	// Code for native runnable that converts a value of any type to a bool.

	Value v = peek(0);  // Keep value in stack so it still has at least one reference

	Value t = NewValue(v.IsTruthy());

	pop(); // remove reference to v
	push(t);
}

void Interpreter::NativeConvertToStr() {
	// Code for native runnable that converts a value of any type to a string.

	Value v = peek(0);  // Keep value in stack so it still has at least one reference

	Value t = NewObject(v.ToString());
	pop(); // remove reference to v
	
	push(t);
}


void Interpreter::NativeTypeOf() {
	// Code for native runnable that prints the datatype of a value.

	Value v = peek(0);  // Keep value in stack so it still has at least one reference

	std::string s;
	switch (v.GetType()) {
		case Value::NONE_T:	s = "NONE";		break;
		case Value::NUM_T:	s = "NUMBER";	break;
		case Value::BOOL_T: s = "BOOL";		break;

		case Value::OBJECT_T: {
			ObjectValue* o = v.GetObjectValue();
			switch (o->GetType())
			{
				case ObjectValue::STRING_T:		s = "STRING";	break;
				case ObjectValue::RUNNABLE_T:	s = "RUNNABLE";	break;
				case ObjectValue::NATIVE_T:		s = "NATIVE";	break;
			}
		}
	}

	Value t = NewObject(s);
	
	pop(); // remove reference to v
	push(t);
}


void Interpreter::DefineNative(std::string name, uint8_t arity, NativeRunnable run) {
	AddGlobal(name, NewObject(new NativeValue(name, arity, run)));
}


Interpreter::Interpreter(RunnableValue* body) {
	this->body = body;
	this->objects = nullptr;
	stack.count = 0;

	globals = std::unordered_map<std::string, Value>();


	// Define native functions
	DefineNative("input",			1, &Interpreter::NativeInput);
	DefineNative("print",			1, &Interpreter::NativePrint);
	
	DefineNative("ReadFromFile",	1, &Interpreter::NativeReadFromFile);
	DefineNative("WriteToFile",		2, &Interpreter::NativeWriteToFile);
	DefineNative("EmptyFile",		1, &Interpreter::NativeEmptyFile);

	DefineNative("Number",			1, &Interpreter::NativeConvertToNum);
	DefineNative("Boolean",			1, &Interpreter::NativeConvertToBool);
	DefineNative("String",			1, &Interpreter::NativeConvertToStr);
	DefineNative("Type",			1, &Interpreter::NativeTypeOf);
}

Interpreter::~Interpreter() {
	if (objects == nullptr) return;
	
	ObjectValue* v = objects;

	// Free ObjectValues
	while (v != nullptr) {
		ObjectValue* next = v->GetNext();
		delete v;
		v = next;
	}
}

int Interpreter::interpret() {
	while (!(CurrentChunk()->IsAtEnd())) {
		try {
			RunCommand();
		}
		catch (ExitCode e) {
			return e;
		}
	}

	return 0;
}

void Interpreter::RunCommand() {
	// Run a single command

#ifdef DEBUG_TRACE_STACK
	int offset = CurrentChunk()->GetOffset();
#endif // DEBUG_TRACE_STACK


// Arithmetic operations + - * / on numbers
#define BINARY_NUM_OP(op)  {\
	Value b = peek(0); \
	Value a = peek(1); \
	if (b.GetType() == Value::NUM_T && a.GetType() == Value::NUM_T){\
		pop();\
		pop();\
		float n1 = GetNumValue(a); \
		float n2 = GetNumValue(b); \
		\
		Value v = NewValue(n1 op n2);\
		push(v);\
	} else {\
		error(TYPE_ERROR, "Can't perform this operation on a non-number");\
	}\
}


// Comparison operations == != <= >= < > on numbers
#define BINARY_COMP_OP(op) {\
	Value b = peek(0); \
	Value a = peek(1); \
	if (b.GetType() == Value::NUM_T && a.GetType() == Value::NUM_T){\
		pop(); \
		pop(); \
		float n1 = GetNumValue(a); \
		float n2 = GetNumValue(b); \
		\
		Value v = NewValue((bool)(n1 op n2));\
		push(v);\
	} else {\
		error(TYPE_ERROR, "Can't perform this operation on a non-number");\
	}\
}


// Bitwise operations & | ^ >> << on integer values
#define BINARY_BIT_OP(op) {\
	Value b = peek(0); \
	Value a = peek(1); \
	if (IsIntegerValue(b) && IsIntegerValue(a)){\
		pop(); \
		pop(); \
		\
		int n1 = GetNumValue(a); \
		int n2 = GetNumValue(b); \
		\
		Value v = NewValue((float)(n1 op n2));\
		push(v); \
	} else {\
		error(TYPE_ERROR, "Can't perform this operation on a non-integer");\
	}\
}


// Variable assignment operations on numbers	+= -= *= /= 
#define BINARY_ASSIGN_OP(a, op, IsPlus) {\
\
	Value b = peek(0); \
	if (a->GetType() == Value::NUM_T && b.GetType() == Value::NUM_T){\
		pop(); \
		a->SetValue(GetNumValue(*a) op GetNumValue(b));\
		Value v = *a; \
		push(v); \
	} else {\
		std::string msg = "Can only perform this operation on two numbers"; \
		if (IsPlus) msg += " or two strings";\
		\
		error(TYPE_ERROR, msg);\
	}\
}


// Variable assignment operations on integer values &= |= ^\ >>= <<=
#define BINARY_BIT_ASSIGN_OP(a, op) {\
\
	Value b = peek(0); \
	\
	if (IsIntegerValue(*a) && IsIntegerValue(b) ){\
		pop(); \
		a->SetValue((float)((int)GetNumValue(*a) op (int)GetNumValue(b))); \
		Value v = *a;\
		push(v); \
	} else {\
		error(TYPE_ERROR, "Can't perform bitwise operations on non-integer types");\
	}\
}

	uint8_t opcode = CurrentChunk()->advance();
	switch (opcode)
	{
		case OP_NEWLINE:	break; 

		case OP_CONSTANT: {
			Value v = CurrentChunk()->ReadConstant(CurrentChunk()->advance());
			push(v);
			break;
		}

		case OP_POP:		pop();	break;

		case OP_NONE: {
			Value v = NewValue();
			push(v);
			break;
		}

		case OP_TRUE: {
			Value v = NewValue(true);
			push(v);
			break;
		}
		case OP_FALSE: {
			Value v = NewValue(false);
			push(v);
			break;
		}

		case OP_NEGATE: {
			Value a = peek(0);

			if (a.GetType() == Value::NUM_T) {
				float n = GetNumValue(a);
				Value v = NewValue(-n);

				pop();
				push(v);
			}
			else {
				error(TYPE_ERROR, "Negating a non-number type");
			}
			
			break;
		}

		case OP_NOT: {
			Value a = peek(0);
			Value::datatype t = a.GetType();
			switch (t) {
				case Value::NUM_T: {
					pop();

					float n = GetNumValue(a);
					Value v = NewValue((float)(~(int)n));
					if (n / 1 == n)	push(v);
					else error(TYPE_ERROR, "Can't perform bitwise operation on a non-integer");
					break;
				}
				case Value::BOOL_T: {
					pop();

					bool b = GetBoolValue(a);

					Value v = NewValue(!b);
					push(v);
					break;
				}
			}
			break;
		}

		case OP_ADD: {
			switch (peek(0).GetType()) {
				case Value::NUM_T:	BINARY_NUM_OP(+);	break;
				case Value::OBJECT_T: {

					std::string msg = "Can only perform this operation on two numbers or two strings";

					Value v2 = peek(0);
					Value v1 = peek(1);
					StrValue* a = ExtractStrValue(&v1, msg);
					StrValue* b = ExtractStrValue(&v2, msg);

					Value v = NewObject((std::string&)(*a + *b));
					
					pop();  // remove reference to v2
					pop();  // remove reference to v1 

					push(v);
					break;
				}
				default:
					error(TYPE_ERROR, "Can only use the '+' operator between two numbers or two strings");
			}

			break;
		}
		case OP_SUB:			BINARY_NUM_OP(-);	break;
		case OP_MULTIPLY: 		BINARY_NUM_OP(*);	break;
		case OP_DIVIDE:			BINARY_NUM_OP(/);	break;

		case OP_BIT_AND:		BINARY_BIT_OP(&);	break;
		case OP_BIT_OR:			BINARY_BIT_OP(|);	break;
		case OP_BIT_XOR:		BINARY_BIT_OP(^);	break;

		case OP_SHIFT_LEFT:		BINARY_BIT_OP(<<);	break;
		case OP_SHIFT_RIGHT:	BINARY_BIT_OP(>>);	break;

		case OP_EQUALS: {
			switch (peek(0).GetType()) {
				case Value::NUM_T: BINARY_COMP_OP(== ); break;
				case Value::BOOL_T: {
					Value v2 = pop();
					Value v1 = peek(0);

					if (v1.GetType() != Value::BOOL_T) {
						pop();

						Value v = NewValue(false);
						push(v);
						break;
					}
					Value v = NewValue(v1.GetBool() == v2.GetBool());
					push(v);
					break;
				}

				case Value::NONE_T: {
					Value v2 = pop();
					Value v1 = peek(0);

					Value v = NewValue(v2.IsNone());
					
					pop(); // delete reference to v1
					push(v);
					break;
				}

				case Value::OBJECT_T: {
					ObjectValue* o2 = peek(0).GetObjectValue();

					Value IsEqual;
					if (!peek(1).IsObject()) {
						IsEqual = NewValue(false);

						pop(); // delete reference to o2
						pop();
						push(IsEqual);
						break;
					}

					ObjectValue* o1 = peek(1).GetObjectValue();
					if (o1->GetType() != o2->GetType()) {
						IsEqual = NewValue(false);
						
					}
					else {
						// If type and string are equal, so are the values
						IsEqual = NewValue((o1->ToString() == o2->ToString()) && (o1->GetType() == o2->GetType()));
					}

					pop(); // delete reference to o2
					pop(); // delete reference to o1
					break;
				}
			}
			break;
		}
		case OP_LESS:		BINARY_COMP_OP(< ); break;
		case OP_GREATER:	BINARY_COMP_OP(>); break;

		case OP_DEFINE_GLOBAL: {
			uint8_t IdIndex = CurrentChunk()->advance(); // Index of identifier in constants table

			std::string identifier = GetConstantStr(IdIndex);

			if (IsDefinedGlobal(identifier)) {
				if (globals[identifier].IsObject() && (globals[identifier].GetObjectValue())->IsNative()) {
					error(REDECLARED_RAT, "identifier '" + identifier + "' is reserved for a native function, " +
						"and cannot be a variable or runnable's name");
				}
				error(REDECLARED_RAT, "rat with the name '" + identifier + "' already exists");
			}

			Value v = peek(0);
			AddGlobal(identifier, v);
			pop();

			break;
		}

		case OP_SET_GLOBAL: {
			uint8_t IdIndex = CurrentChunk()->advance();  // Index of identifier in constants table
			std::string identifier = GetConstantStr(IdIndex);

			Value v = peek(0); // want to keep value on the stack in case the assignment is part of an expression

			if (!IsDefinedGlobal(identifier)) error(UNDEFINED_RAT, "Setting value to an undefined rat");

			if (globals[identifier].IsObject()) {
				ObjectValue* o = globals[identifier].GetObjectValue();

				if (o->GetType() == ObjectValue::RUNNABLE_T) error(TYPE_ERROR, "Can't reassign a runnable");
				else if (o->GetType() == ObjectValue::NATIVE_T) error(TYPE_ERROR, "Can't set a value to a native runnable");

				// Remove reference
				bool deleted = o->DeleteReference();
				if (deleted) {
					RemoveObject(o);
					globals[identifier].SetValue(nullptr);
				}
			}
			globals[identifier] = v;
			if (v.IsObject()) {
				v.GetObjectValue()->AddReference();
			}
			break;
		}
						  
		case OP_GET_GLOBAL: {
			Value* var = FindGlobal();

			push(*var);
			break;
		}

		case OP_GET_LOCAL: {
			
			Value* var = FindLocal();

			push(*var);
			break;
		}

		case OP_SET_LOCAL: {
			uint8_t FrameSlot = CurrentChunk()->advance();

			Value v = this->stack.stk[this->body->GetFrameStart() + FrameSlot + 1];
			if (v.IsObject()) {

				// Remove reference
				bool deleted = v.GetObjectValue()->DeleteReference();
				if (deleted) {
					RemoveObject(v.GetObjectValue());
					v.SetValue(nullptr);
				}
			}

			this->stack.stk[this->body->GetFrameStart() + FrameSlot + 1] = peek(0);
			if (peek(0).IsObject()) {
				peek(0).GetObjectValue()->AddReference();
			}
			break;
		}

		case OP_INC_GLOBAL: {
			Value* var = FindGlobal();

			if (var->GetType() != Value::NUM_T) {
				error(TYPE_ERROR, "Can't increment a non-number value");
			}

			var->SetValue(GetNumValue(*var) + 1);
			
			push(*var);
			break;
		}

		case OP_INC_LOCAL: {
			Value* var = FindLocal();

			if (var->GetType() != Value::NUM_T) {
				error(TYPE_ERROR, "Can't increment a non-number value");
			}
			var->SetValue(GetNumValue(*var) + 1);

			push(*var);
			break;
		}



		case OP_DEC_GLOBAL: {
			Value* var = FindGlobal();

			if (var->GetType() != Value::NUM_T) {
				error(TYPE_ERROR, "Can't decrement a non-number value");
			}

			var->SetValue(GetNumValue(*var) - 1);
			push(*var);

			break;
		}

		case OP_DEC_LOCAL: {
			Value* var = FindLocal();

			if (var->GetType() != Value::NUM_T) {
				error(TYPE_ERROR, "Can't decrement a non-number value");
			}

			var->SetValue(GetNumValue(*var) - 1);
			push(*var);

			break;
		}

		case OP_ADD_ASSIGN_GLOBAL: {
			switch (peek(0).GetType()) {
				case Value::OBJECT_T: {
					ObjectValue* o = peek(0).GetObjectValue();

					if (o->IsString()) {
						std::string ErrorMsg = "Can only perform this operation on two strings or two numbers";

						Value v = peek(0);
						StrValue* b = ExtractStrValue(&v, ErrorMsg);
						StrValue* a = ExtractStrValue(FindGlobal(), ErrorMsg);

						a->SetValue((std::string&)(*a + *b)); // no need to change references to a,
						//the ObjectValue is the same
						
						
						pop(); // delete reference to v
						
						Value v2 = NewObject(a);
						push(v2);
					}
					
					break;
				}

				default:{
					Value* a = FindGlobal();
					BINARY_ASSIGN_OP(a, +, false); // macro will report a type error
					break;
				}
			}
			break;
		}
		
		case OP_ADD_ASSIGN_LOCAL: {
			switch (peek(0).GetType()) {
				case Value::OBJECT_T: {

					std::string msg = "Can only perform this operation on two numbers or two strings";
					Value v = peek(0); // don't want to remove reference yet
					StrValue* b = ExtractStrValue(&v, msg);

					Value* va = FindLocal();
					StrValue* a = ExtractStrValue(va, msg);

					a->SetValue((std::string&)(*a + *b)); // no need to change references to a,
					//the ObjectValue is the same
					
					va->SetValue(a);

					pop(); // remove reference to v
					push(*va);
					break;
				}

				default: {
					Value* a = FindLocal();
					BINARY_ASSIGN_OP(a, +, false);
					break;
				}
			}
			break;
		}



		case OP_SUB_ASSIGN_GLOBAL: {
			Value* a = FindGlobal();
			BINARY_ASSIGN_OP(a, -, false);
			break;
		}

		case OP_MULTIPLY_ASSIGN_GLOBAL: {
			Value* a = FindGlobal();
			BINARY_ASSIGN_OP(a, *, false);
			break;
		}

		case OP_DIVIDE_ASSIGN_GLOBAL: {
			Value* a = FindGlobal();
			BINARY_ASSIGN_OP(a, /, false);
			break;
		}


		case OP_SUB_ASSIGN_LOCAL: {
			Value* a = FindLocal();
			BINARY_ASSIGN_OP(a, -, false);
			break;
		}

		case OP_MULTIPLY_ASSIGN_LOCAL: {
			Value* a = FindLocal();
			BINARY_ASSIGN_OP(a, *, false);
			break;
		}

		case OP_DIVIDE_ASSIGN_LOCAL: {
			Value* a = FindLocal();
			BINARY_ASSIGN_OP(a, / , false);
			break;
		}




		case OP_BIT_AND_ASSIGN_GLOBAL: {
			Value* a = FindGlobal();
			BINARY_BIT_ASSIGN_OP(a, &);
			break;
		}

		case OP_BIT_OR_ASSIGN_GLOBAL: {
			Value* a = FindGlobal();
			BINARY_BIT_ASSIGN_OP(a, |);
			break;
		}

		case OP_BIT_XOR_ASSIGN_GLOBAL: {
			Value* a = FindGlobal();
			BINARY_BIT_ASSIGN_OP(a, ^);
			break;
		}


		case OP_BIT_AND_ASSIGN_LOCAL: {
			Value* a = FindLocal();
			BINARY_BIT_ASSIGN_OP(a, &);
			break;
		}

		case OP_BIT_OR_ASSIGN_LOCAL: {
			Value* a = FindLocal();
			BINARY_BIT_ASSIGN_OP(a, | );
			break;
		}

		case OP_BIT_XOR_ASSIGN_LOCAL: {
			Value* a = FindLocal();
			BINARY_BIT_ASSIGN_OP(a, ^);
			break;
		}




		case OP_SHIFTL_ASSIGN_GLOBAL: {
			Value* a = FindGlobal();
			BINARY_BIT_ASSIGN_OP(a, <<);
			break;
		}

		case OP_SHIFTR_ASSIGN_GLOBAL: {
			Value* a = FindGlobal();
			BINARY_BIT_ASSIGN_OP(a, >>);
			break;
		}


		case OP_SHIFTL_ASSIGN_LOCAL: {
			Value* a = FindLocal();
			BINARY_BIT_ASSIGN_OP(a, << );
			break;
		}

		case OP_SHIFTR_ASSIGN_LOCAL: {
			Value* a = FindLocal();
			BINARY_BIT_ASSIGN_OP(a, >> );
			break;
		}

		case OP_JUMP_IF_TRUE: {
			uint8_t JumpHighByte = this->CurrentChunk()->advance();
			uint8_t JumpLowByte = this->CurrentChunk()->advance();

			if (peek(0).IsTruthy()) {
				short distance = (short)(JumpHighByte << 8) + (short)(JumpLowByte);
				this->CurrentChunk()->MoveIp(distance);
			}
			break;
		}
		case OP_JUMP_IF_FALSE: {
			uint8_t JumpHighByte = this->CurrentChunk()->advance();
			uint8_t JumpLowByte = this->CurrentChunk()->advance();

			if (!(peek(0).IsTruthy())) {
				short distance = (short)(JumpHighByte << 8) + (short)(JumpLowByte);
				this->CurrentChunk()->MoveIp(distance);
			}
			break;
		}
		case OP_JUMP: {
			uint8_t JumpHighByte =	this->CurrentChunk()->advance();
			uint8_t JumpLowByte =	this->CurrentChunk()->advance();

			short distance = (short)(JumpHighByte << 8) + (short)(JumpLowByte);
			this->CurrentChunk()->MoveIp(distance);
			break;
		}
		case OP_LOOP: {
			uint8_t JumpHighByte =	this->CurrentChunk()->advance();
			uint8_t JumpLowByte =	this->CurrentChunk()->advance();

			short distance = (short)(JumpHighByte << 8) + (short)(JumpLowByte);
			this->CurrentChunk()->MoveIp(-distance);
			break;
		}

		case OP_REPEAT: {
			Value v = peek(0);
			if (!IsIntegerValue(v)) error(TYPE_ERROR, "Can only use positive integer values as the operand to 'repeat'");
			
			int n = GetNumValue(v);

			if (n <= 0)  error(TYPE_ERROR, "Can only use positive integer values as the operand to 'repeat'");
			break;
		}

		case OP_END_REPEAT: {
			// Decrease repeat operand by 1 and loop back (or not)
			Value v = pop();
			if (!IsIntegerValue(v)) error(INTERNAL_ERROR, "");

			v.SetValue(v.GetNum() - 1);

			int n = GetNumValue(v);

			if (n == 0) {
				CurrentChunk()->MoveIp(4);  // skip over 'op_loop' instruction
			}
			else {
				push(v);
			}

			break;
		}

		case OP_DEFINE_RUNNABLE: {
			uint8_t index = CurrentChunk()->advance();		// Index of runnable identifier in constants table

			uint8_t linesnum = CurrentChunk()->advance();   // Number of lines in the runnable
			//linesnum can be discarded for now- meant to be used only when reporting errors

			Value v = CurrentChunk()->ReadConstant(index);
			if (!v.IsObject()) error(INTERNAL_ERROR, "");
			
			ObjectValue* o = v.GetObjectValue();
			if (!o->IsRunnable()) error(INTERNAL_ERROR, "");

			RunnableValue* runnable = (RunnableValue *)o;

			AddGlobal(runnable->GetName(), NewObject(runnable));
			break;
		}


		case OP_CALL: {
			Value* called = FindGlobal();

			if (called->IsObject() && called->GetObjectValue()->IsRunnable()) {
				RunnableValue* runnable = ((RunnableValue*)called->GetObjectValue());

				uint8_t FrameIndex = this->stack.count - runnable->GetArity() - 1;
				// current capacity, minus arguments and identifier


				// Copy the runnable, don't invoke it directly. This way, recursion is allowed
				Chunk* c = new Chunk(runnable->GetChunk());
				RunnableValue* NewBody = new RunnableValue(runnable, c, this->body, FrameIndex);

				SetBody(NewBody);
			}
			else {
				error(TYPE_ERROR, "Can't call an object that isn't a runnable");
			}
			break;
		}

		case OP_CALL_NATIVE: {
			Value* called = FindGlobal();
			uint8_t arity = CurrentChunk()->advance();

			if (called->IsObject() && called->GetObjectValue()->IsNative()) {
				NativeValue* nv = (NativeValue *)called->GetObjectValue();
				if (arity != nv->GetArity()) {
					error(TYPE_ERROR, nv->ToString() + " called with " +
						std::to_string(arity) + " arguments, but accepts " + std::to_string(nv->GetArity()));
				}
				NativeRunnable n = nv->GetRunnable();
				(this->*n)(); // Call native runnable
			}
			else {
				error(TYPE_ERROR, "Can't call an object that isn't a runnable");
			}

			if (peek(0).IsObject()) peek(0).GetObjectValue()->AddReference(); 
			// so it doesn't get deleted when popping before call frame
			
			Value ReturnValue = pop();

			pop(); // remove runnable from stack
			
			push(ReturnValue);
			if (ReturnValue.IsObject()) peek(0).GetObjectValue()->DeleteReference();

			break;
		}

		case OP_RETURN: {
			if (peek(0).IsObject()) {
				peek(0).GetObjectValue()->AddReference(); 
				// Add reference to the return value so it won't get deleted now
				// when it will be popped before the call frame
			}
			Value ReturnVal = pop();

			for (int i = this->stack.count; i > this->body->GetFrameStart(); i--) {
				pop();  // pop frame off the stack
			}

			push(ReturnVal); // Push return value back on the stack so it will be available for use
			
			if (ReturnVal.IsObject()) ReturnVal.GetObjectValue()->DeleteReference();
			// Delete reference that was added earlier

			SetBody(this->body->GetEnclosing());

			if (this->body == nullptr) error(RETURN_FROM_SCRIPT, "Can't return from the global script");

			break;
		}

		default:
			error(UNRECOGNIZED_OPCODE, "Unrecognized opcode " + opcode);
			break;
	}

#ifdef DEBUG_TRACE_STACK
	std::cout << TraceStack(offset);
#endif
}


Chunk* Interpreter::CurrentChunk() {
	return this->body->GetChunk();
}

void Interpreter::SetBody(RunnableValue* body) {
	this->body = body;
}


Value& Interpreter::pop() {
	// Pop value from the vm stack
	if (this->stack.count <= 0) {
		error(STACK_UNDERFLOW, "Popping from empty stack");
	}

	stack.count--;

	Value i = stack.stk[stack.count];

	if (i.IsObject()) {
		// Delete reference to object
		bool deleted = i.GetObjectValue()->DeleteReference();
		if (deleted) {
			RemoveObject(i.GetObjectValue());
			stack.stk[stack.count].SetValue(nullptr);
		}
	}

	return stack.stk[stack.count];
}

void Interpreter::RemoveObject(ObjectValue* o) {
	// Remove the object form the linked list and free it's memory
	if (o == nullptr) return;

	if (this->objects == o) {
		this->objects = o->GetNext();

#ifdef DEBUG_GC_INFO
		std::cout << "[Garbage collector] Deallocated '" + o->ToString() + "'\n";
#endif
		delete o;
		return;
	}

	ObjectValue* curr = this->objects;
	while (curr != nullptr && curr->GetNext() != o) {
		curr = curr->GetNext();
	}

	if (curr != nullptr) {
		curr->SetNext(curr->GetNext()->GetNext());
	}
	
#ifdef DEBUG_GC_INFO
	std::cout << "[Garbage collector] Deallocated '" + o->ToString() + "'\n";
#endif

	delete o;
}



void Interpreter::push(Value& value) {
	// Push a value to the vm stack
	if (stack.count == StackSize) error(STACK_OVERFLOW, "Stack limit exceeded");
	stack.stk[stack.count++] = value;

	if (value.IsObject()) value.GetObjectValue()->AddReference();

	return;
}

Value& Interpreter::peek(int depth) {
	if (depth > stack.count - 1) error(STACK_UNDERFLOW, "Can't peek so deep into stack");
	return this->stack.stk[stack.count - 1 - depth];
}





StrValue* Interpreter::ExtractStrValue(Value* v, std::string& ErrorMsg) {
	// Return the StrValue that v holds, if it does.
	// If v is not a StrValue, raise an error

	if (v->GetType() != Value::OBJECT_T) error(TYPE_ERROR, ErrorMsg);

	ObjectValue* o = v->GetObjectValue();

	if (o->GetType() != ObjectValue::STRING_T) error(TYPE_ERROR, ErrorMsg);
	return (StrValue*)o;
}



std::string Interpreter::GetConstantStr(uint8_t index) {
	// Get the string at index 'index' in the chunks constants table

	Value v = CurrentChunk()->ReadConstant(index);
	StrValue* s = (StrValue *)v.GetObjectValue();
	return s->GetValue();
}

bool Interpreter::GetConstantBool(uint8_t index) {
	// Get the boolean at index 'index' in the chunks constants table
	Value v = CurrentChunk()->ReadConstant(index);
	return v.GetBool();
}

float Interpreter::GetConstantNum(uint8_t index) {
	// Get the number at index 'index' in the chunks constants table
	Value v = CurrentChunk()->ReadConstant(index);
	return v.GetNum();
}


Value *Interpreter::FindLocal() {
	uint8_t FrameSlot = CurrentChunk()->advance();
	return &(this->stack.stk[this->body->GetFrameStart() + FrameSlot + 1]);
}

Value *Interpreter::FindGlobal() {
	uint8_t IdIndex = CurrentChunk()->advance();
	std::string identifier = GetConstantStr(IdIndex);

	if (IsDefinedGlobal(identifier)) return &(this->globals[identifier]);

	error(UNDEFINED_RAT, "Undefined rat '" + identifier + "' ");
}

void Interpreter::AddGlobal(std::string& name, Value value) {
	this->globals.insert({ name, value });
	if (value.IsObject()) {
		value.GetObjectValue()->AddReference();
	}
}

bool Interpreter::IsDefinedGlobal(std::string& identifier ) {
	return this->globals.find(identifier) != this->globals.end();
}


std::string Interpreter::TraceStack(int CodeOffset) {
	// Print the stack contents to the screen
	
	std::string trace = "";
	for (uint8_t i = 0; i < this->stack.count; i++) {
		trace += ("[ " + this->stack.stk[i].ToString() + " ]\t");
	}
	trace += '\n';
	return (std::to_string(CodeOffset) + "\t" + trace);
}

void Interpreter::error(ExitCode e, std::string msg) {
	int line = CurrentChunk()->CountLines(false);
	std::string bodyname = "<Script>";

	if (body->GetEnclosing()) { // in a runnable
		RunnableValue* script = body->GetEnclosing();
		line += script->GetChunk()->CountLines(body->ToString());
		bodyname = body->ToString();
	}

	std::cerr << "[Runtime error in " + bodyname + " in line " << line << "]: " << msg << "\n";
	throw e;
}