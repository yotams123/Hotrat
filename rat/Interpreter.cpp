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
	Value v = NewObject(res);
	return v;
}

Value Interpreter::NewObject(ObjectValue* o) {

	Value v = Value(o);
	ObjectValue* obj = v.GetObjectValue();

	obj->SetNext(this->objects);
	this->objects = obj;
	return v;
}


Value NewValue() {
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
	std::string str;
	std::getline(std::cin, str);

	push(NewObject(str));
}

void Interpreter::NativePrint() {
	Value v = pop();
	std::cout << v.ToString() + "\n";
	push(NewValue());  // none
}

void Interpreter::NativeReadFromFile() {
	// reads entire file and returns a strvalue

	Value v = pop();
	if (!v.IsObject() || !(v.GetObjectValue())->IsString()) {
		error(TYPE_ERROR, "First argument to ReadFile must be a string");
	}

	std::string FileName = v.GetObjectValue()->ToString();
	LPCSTR filename = FileName.c_str();

	HANDLE handle = CreateFileA(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) error(INTERNAL_ERROR, "Error opening the file\t\tare you sure the path is correct?");
	
	DWORD NumToReadH;
	DWORD NumToReadL = GetFileSize(handle, &NumToReadH);
	
	const int toread = ((NumToReadH << 8) | (NumToReadL));
	char* buff = new char[toread + 1];

	bool success = ReadFile(handle, buff, toread, NULL, NULL);
	if (success == FALSE) error(INTERNAL_ERROR, "Error reading the file");
	buff[toread] = 0;

	std::string strbuf = std::string(buff);

	delete[] buff;
	buff = nullptr;

	CloseHandle(handle);
	push(NewObject(strbuf));
}

void Interpreter::NativeWriteToFile() {
	// appends new text to end of file

	Value BuffValue = pop();
	if (!BuffValue.IsObject() || !BuffValue.GetObjectValue()->IsString()) error(TYPE_ERROR,
		"Second argument to WriteToFile must be a string value");

	std::string buff = BuffValue.GetObjectValue()->ToString();

	Value FileValue = pop();
	if (!FileValue.IsObject() || !(FileValue.GetObjectValue()->IsString())) {
		error(TYPE_ERROR, "Second argument to WriteToFile must be a string value");
	}

	std::string filename = FileValue.GetObjectValue()->ToString();
	LPCSTR CFileName = filename.c_str();

	HANDLE handle = CreateFileA(CFileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE) {
		error(INTERNAL_ERROR, "Couldn't open file\t\tare you sure the path is correct?");
	}

	SetFilePointer(handle, 0, NULL, FILE_END);

	bool success = WriteFile(handle, buff.c_str(), buff.size(), NULL, NULL);
	if (!success) {
		error(INTERNAL_ERROR, "Error writing to file");
	}

	CloseHandle(handle);

	push(NewValue());  // none
}

void Interpreter::NativeEmptyFile() {
	Value FileValue = pop();
	std::string msg = "EmptyFile's argument must be a string value";

	StrValue * filestr = ExtractStrValue(&FileValue, msg);
	std::string filename = filestr->GetValue();

	HANDLE handle = CreateFileA(filename.c_str(), GENERIC_WRITE, NULL, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!handle) error(INTERNAL_ERROR, "Couldn't open file\t\tare you sure the path is correct?");

	CloseHandle(handle);
	push(NewValue()); // none
}

void Interpreter::NativeConvertToNum() {
	Value v = pop();
	float num;
	switch (v.GetType())
	{
		case Value::BOOL_T:  num = (v.GetBool() ? 1 : 0); break;
		case Value::NUM_T:	 push(v);	return;
		case Value::NONE_T:	 num = 0;   return;

		default: {
			std::string errormsg = "Value given to 'Number()' must be of valid type";
			StrValue* s = ExtractStrValue(&v, errormsg);

			std::string str = s->ToString();
			for (int i = 0; i < str.size(); i++) if (!isdigit(str[i])) error(TYPE_ERROR, "Input to Number(), if a string, must be a valid");

			num = std::stof(s->ToString());
			break;
		}
	}

	push(NewValue(num));
}

void Interpreter::NativeConvertToBool() {
	Value v = pop();
	if (v.IsObject() && v.GetObjectValue()->IsString()) {
		std::string m = "";
		std::string s = ExtractStrValue(&v, m)->ToString();

		if (s == "false") {
			push(NewValue(false));
			return;
		}
	}

	push(NewValue(v.IsTruthy()));
}

void Interpreter::NativeConvertToStr() {
	Value v = pop();
	push(NewObject(v.ToString()));
}


void Interpreter::NativeTypeOf() {
	Value v = pop();

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

	push(NewObject(s));
}


void Interpreter::DefineNative(std::string name, uint8_t arity, NativeRunnable run) {
	AddGlobal(name, NewObject(new NativeValue(name, arity, run)));
}


Interpreter::Interpreter(RunnableValue* body) {
	this->body = body;
	this->objects = nullptr;
	stack.count = 0;

	globals = std::unordered_map<std::string, Value>();

	DefineNative("input",			0, &Interpreter::NativeInput);
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
#ifdef DEBUG_TRACE_STACK
	int offset = CurrentChunk()->GetOffset();
#endif // DEBUG_TRACE_STACK

#define BINARY_NUM_OP(op)  {\
	Value b = pop(); \
	Value a = pop(); \
	if (b.GetType() == Value::NUM_T && a.GetType() == Value::NUM_T){\
		\
		float n1 = GetNumValue(a); \
		float n2 = GetNumValue(b); \
		push(NewValue(n1 op n2));\
	} else {\
		error(TYPE_ERROR, "Can't perform this operation on a non-number");\
	}\
}

#define BINARY_COMP_OP(op) {\
	Value b = pop(); \
	Value a = pop(); \
	if (b.GetType() == Value::NUM_T && a.GetType() == Value::NUM_T){\
		\
		float n1 = GetNumValue(a); \
		float n2 = GetNumValue(b); \
		push(NewValue((bool)(n1 op n2)));\
	} else {\
		error(TYPE_ERROR, "Can't perform this operation on a non-number");\
	}\
}

#define BINARY_BIT_OP(op) {\
	Value b = pop(); \
	Value a = pop(); \
	if (IsIntegerValue(b) && IsIntegerValue(a)){\
		\
		int n1 = GetNumValue(a); \
		int n2 = GetNumValue(b); \
		push(NewValue((float)(n1 op n2))); \
	} else {\
		error(TYPE_ERROR, "Can't perform this operation on a non-integer");\
	}\
}

#define BINARY_ASSIGN_OP(a, op, IsPlus) {\
\
	Value b = pop(); \
	if (a->GetType() == Value::NUM_T && b.GetType() == Value::NUM_T){\
		a->SetValue(GetNumValue(*a) op GetNumValue(b));\
		push(*a); \
	} else {\
		std::string msg = "Can only perform this operation on two numbers"; \
		if (IsPlus) msg += " or two strings";\
		\
		error(TYPE_ERROR, msg);\
	}\
}

#define BINARY_BIT_ASSIGN_OP(a, op) {\
\
	Value b = pop(); \
	\
	if (IsIntegerValue(*a) && IsIntegerValue(b) ){\
		a->SetValue((float)((int)GetNumValue(*a) op (int)GetNumValue(b))); \
		push(*a); \
	} else {\
		error(TYPE_ERROR, "Can't perform bitwise operations on non-integer types");\
	}\
}

	uint8_t opcode = CurrentChunk()->advance();
	switch (opcode)
	{
		case OP_NEWLINE:	std::cout << "\n\n\n";	break; 

		case OP_CONSTANT:	push(CurrentChunk()->ReadConstant(CurrentChunk()->advance())); break;
		case OP_POP:		pop();	break;

		case OP_NONE: {
			push(Value());
			break;
		}

		case OP_TRUE: {
			push(NewValue(true));
			break;
		}
		case OP_FALSE: {
			push(NewValue(false));
			break;
		}

		case OP_NEGATE: {
			Value a = pop();

			if (a.GetType() == Value::NUM_T) {
				float n = GetNumValue(a);
				push(NewValue(-n));
			}
			else {
				error(TYPE_ERROR, "Negating a non-number type");
			}
			
			break;
		}

		case OP_NOT: {
			Value a = pop();
			Value::datatype t = a.GetType();
			switch (t) {
				case Value::NUM_T: {
					float n = GetNumValue(a);
					if (n / 1 == n)	push(NewValue((float)~(int)n));
					else error(TYPE_ERROR, "Can't perform bitwise operation on a non-integer");
					break;
				}
				case Value::BOOL_T: {
					bool b = GetBoolValue(a);
					push(NewValue(!b));
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

					Value v2 = pop();
					Value v1 = pop();
					StrValue* a = ExtractStrValue(&v1, msg);
					StrValue* b = ExtractStrValue(&v2, msg);

					push(NewObject((std::string&)(*a + *b)));
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
					Value v1 = pop();
					Value v2 = pop();

					if (v2.GetType() != Value::BOOL_T) {
						push(NewValue(false));
						break;
					}
					push(NewValue(v1.GetBool() == v2.GetBool()));
					break;
				}

				case Value::NONE_T: {
					Value v1 = pop();
					Value v2 = pop();

					push(NewValue(v2.IsNone()));
					break;
				}

				case Value::OBJECT_T: {
					ObjectValue* o1 = pop().GetObjectValue();
					ObjectValue* o2 = pop().GetObjectValue();

					if (o1->GetType() != o2->GetType()) {
						push(NewValue(false));
					}
					else {
						push(NewValue(o1->ToString() == o2->ToString()));
					}

					break;
				}
			}
			break;
		}
		case OP_LESS:		BINARY_COMP_OP(< ); break;
		case OP_GREATER:	BINARY_COMP_OP(>); break;

		case OP_DEFINE_GLOBAL: {
			uint8_t IdIndex = CurrentChunk()->advance();

			std::string identifier = GetConstantStr(IdIndex);

			if (IsDefinedGlobal(identifier)) {
				if (globals[identifier].IsObject() && (globals[identifier].GetObjectValue())->IsNative()) {
					error(REDECLARED_RAT, "identifier '" + identifier + "' is reserved for a native function, " +
						"and cannot be a variable or runnable's name");
				}
				error(REDECLARED_RAT, "rat with the name '" + identifier + "' already exists");
			}

			Value v = pop();
			AddGlobal(identifier, v);
			break;
		}

		case OP_SET_GLOBAL: {
			uint8_t IdIndex = CurrentChunk()->advance();
			std::string identifier = GetConstantStr(IdIndex);

			Value v = peek(0); // want to keep value on the stack in case the assignment is part of an expression

			if (!IsDefinedGlobal(identifier)) error(UNDEFINED_RAT, "Setting value to an undefined rat");

			if (globals[identifier].IsObject()) {
				ObjectValue* o = globals[identifier].GetObjectValue();

				if (o->GetType() == ObjectValue::RUNNABLE_T) error(TYPE_ERROR, "Can't reassign a runnable");
				else if (o->GetType() == ObjectValue::NATIVE_T) error(TYPE_ERROR, "Can't set a value to a native runnable");
			}
			globals[identifier] = v;
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
			this->stack.stk[this->body->GetFrameStart() + FrameSlot + 1] = peek(0);

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
			push(var);

			break;
		}

		case OP_ADD_ASSIGN_GLOBAL: {
			switch (peek(0).GetType()) {
				case Value::OBJECT_T: {
					ObjectValue* o = peek(0).GetObjectValue();

					if (o->IsString()) {
						std::string ErrorMsg = "Can only perform this operation on two strings or two numbers";

						Value v = pop();
						StrValue* b = ExtractStrValue(&v, ErrorMsg);
						StrValue* a = ExtractStrValue(FindGlobal(), ErrorMsg);

						a->SetValue((std::string&)(*a + *b));
						push(a);
					}
					
					break;
				}

				default:{
					Value* a = FindGlobal();
					BINARY_ASSIGN_OP(a, +, false);
					break;
				}
			}
			break;
		}
		
		case OP_ADD_ASSIGN_LOCAL: {
			switch (peek(0).GetType()) {
				case Value::OBJECT_T: {

					std::string msg = "Can only perform this operation on two numbers or two strings";
					Value v = pop();
					StrValue* b = ExtractStrValue(&v, msg);

					Value* va = FindLocal();
					StrValue* a = ExtractStrValue(va, msg);

					a->SetValue((std::string&)(*a + *b));
					
					va->SetValue(a);
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
			uint8_t index = CurrentChunk()->advance();
			uint8_t linesnum = CurrentChunk()->advance();  
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

			if (called->IsObject() && ((ObjectValue *)called)->IsRunnable() ){
				RunnableValue* runnable = ((RunnableValue*)called->GetObjectValue());
				uint8_t FrameIndex = this->stack.count - runnable->GetArity() - 1; // current capacity, minus arguments and identifier

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
					error(TYPE_ERROR, "Native called with " +
						std::to_string(arity) + " arguments, but accepts " + std::to_string(nv->GetArity()));
				}
				NativeRunnable n = nv->GetRunnable();
				(this->*n)();
			}
			else {
				error(TYPE_ERROR, "Can't call an object that isn't a runnable");
			}

			Value ReturnValue = pop();
			pop(); // remove runnable from stack
			push(ReturnValue);
			break;
		}

		case OP_RETURN: {
			Value ReturnVal = pop();

			this->stack.count = this->body->GetFrameStart();  // pop frame off the stack

			push(ReturnVal);

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


Value Interpreter::pop() {
	if (this->stack.count <= 0) {
		error(STACK_UNDERFLOW, "Popping from empty stack");
	}

	stack.count--;
	Value i = stack.stk[stack.count];

	return i;
}

void Interpreter::push(Value value) {
	if (stack.count == StackSize) error(STACK_OVERFLOW, "Stack limit exceeded");
	stack.stk[stack.count++] = value;
	return;
}

Value& Interpreter::peek(int depth) {
	if (depth > stack.count - 1) error(STACK_UNDERFLOW, "Can't peek so deep into stack");
	return this->stack.stk[stack.count - 1 - depth];
}





StrValue* Interpreter::ExtractStrValue(Value* v, std::string& ErrorMsg) {
	if (v->GetType() != Value::OBJECT_T) error(TYPE_ERROR, ErrorMsg);

	ObjectValue* o = v->GetObjectValue();

	if (o->GetType() != ObjectValue::STRING_T) error(TYPE_ERROR, ErrorMsg);
	return (StrValue*)o;
}



std::string Interpreter::GetConstantStr(uint8_t index) {
	Value v = CurrentChunk()->ReadConstant(index);
	StrValue* s = (StrValue *)v.GetObjectValue();
	return s->GetValue();
}

bool Interpreter::GetConstantBool(uint8_t index) {
	Value v = CurrentChunk()->ReadConstant(index);
	return v.GetBool();
}

float Interpreter::GetConstantNum(uint8_t index) {
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
}

bool Interpreter::IsDefinedGlobal(std::string& identifier ) {
	return this->globals.find(identifier) != this->globals.end();
}


std::string Interpreter::TraceStack(int CodeOffset) {
	std::string trace = "";
	for (uint8_t i = 0; i < this->stack.count; i++) {
		trace += ("[ " + this->stack.stk[i].ToString() + " ]\t");
	}
	trace += '\n';
	return (std::to_string(CodeOffset) + "\t" + trace);
}

void Interpreter::error(ExitCode e, std::string msg) {
	int line = CurrentChunk()->CountLines(false);
	std::cerr << "[Runtime error in line " << line << "]: " << msg << "\n";
	throw e;
}