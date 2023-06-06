#include "Interpreter.h"

bool IsIntegerValue(Value *f) {
	if (f->GetType() != Value::NUM_T) return false;
	float n = ((NumValue*)f)->GetValue();
	if (n == (int)n) return true;
	return false;
}

Interpreter::Interpreter(RunnableValue* body) {
	this->body = body;
	this->objects = nullptr;
	stack.count = 0;

	globals = std::unordered_map<std::string, Value*>();
}

Interpreter::~Interpreter() {
	if (objects == nullptr) return;
	Value* v = objects;
	while (v != nullptr) {
		Value* next = v->next;
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
	Value *b = pop(); \
	Value *a = pop(); \
	if (b->GetType() == Value::NUM_T && a->GetType() == Value::NUM_T){\
		\
		float n1 = GetNumValue(a); \
		float n2 = GetNumValue(b); \
		push(NewObject(n1 op n2));\
	} else {\
		error(TYPE_ERROR, "Can't perform this operation on a non-number");\
	}\
}

#define BINARY_COMP_OP(op) {\
	Value *b = pop(); \
	Value *a = pop(); \
	if (b->GetType() == Value::NUM_T && a->GetType() == Value::NUM_T){\
		\
		float n1 = GetNumValue(a); \
		float n2 = GetNumValue(b); \
		push(NewObject((bool)(n1 op n2)));\
	} else {\
		error(TYPE_ERROR, "Can't perform this operation on a non-number");\
	}\
}

#define BINARY_BIT_OP(op) {\
	Value *b = pop(); \
	Value *a = pop(); \
	if (IsIntegerValue(b) && IsIntegerValue(a)){\
		\
		int n1 = GetNumValue(a); \
		int n2 = GetNumValue(b); \
		push(NewObject((float)(n1 op n2))); \
	} else {\
		error(TYPE_ERROR, "Can't perform this operation on a non-integer");\
	}\
}

#define BINARY_ASSIGN_OP(op, IsPlus) {\
\
	Value *a = FindGlobal(); \
	Value *b = pop(); \
	if (a->GetType() == Value::NUM_T && b->GetType() == Value::NUM_T){\
		NumValue *na = (NumValue *)a;\
		na->SetValue(GetNumValue(a) op GetNumValue(b));\
		push(na); \
	} else {\
		std::string msg = "Can only perform this operation on two numbers"; \
		if (IsPlus) msg += " or two strings";\
		\
		error(TYPE_ERROR, msg);\
	}\
}

#define BINARY_BIT_ASSIGN_OP(op) {\
\
	Value *a = FindGlobal(); \
	Value *b = pop(); \
	\
	if (IsIntegerValue(a) && IsIntegerValue(b) ){\
		NumValue *num1 = (NumValue *)a; \
		NumValue *num2 = (NumValue *)b; \
		\
		num1->SetValue((int)GetNumValue(a) op (int)GetNumValue(b)); \
		push(num1); \
	} else {\
		error(TYPE_ERROR, "Can't perform bitwise operations on non-integer types");\
	}\
}

	uint8_t opcode = CurrentChunk()->advance();
	switch (opcode)
	{
		case OP_NEWLINE: {
			if (stack.count > 0) std::cout << "\n\n\n";
			break; 
		} 
		case OP_CONSTANT:	push(CurrentChunk()->ReadConstant(CurrentChunk()->advance())); break;
		case OP_POP:		pop();	break;

		case OP_NONE: {
			push(NewObject(nullptr));
			break;
		}

		case OP_TRUE: {
			push(NewObject(true));
			break;
		}
		case OP_FALSE: {
			push(NewObject(false));
			break;
		}

		case OP_NEGATE: {
			Value *a = pop();

			if (a->GetType() == Value::NUM_T) {
				float n = GetNumValue(a);
				push(new NumValue(-n));
			}
			else {
				error(TYPE_ERROR, "Negating a non-number type");
			}
			
			break;
		}

		case OP_NOT: {
			Value *a = pop();
			Value::datatype t = a->GetType();
			switch (t) {
				case Value::NUM_T: {
					float n = GetNumValue(a);
					if (n / 1 == n)	push(new NumValue(~(int)n));
					else error(TYPE_ERROR, "Can't perform bitwise operation on a non-integer");
					break;
				}
				case Value::BOOL_T: {
					bool b = GetBoolValue(a);
					push(NewObject(!b));
					break;
				}
			}
			break;
		}

		case OP_ADD: {
			switch (peek(0)->GetType()) {
				case Value::NUM_T:	BINARY_NUM_OP(+);	break;
				case Value::STRING_T: {
					if (peek(1)->GetType() != Value::STRING_T) error(TYPE_ERROR, "Can only concatenate two strings");
					StrValue* b = (StrValue *)pop();
					StrValue* a = (StrValue *)pop();

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

		case OP_EQUALS:		BINARY_COMP_OP(==); break;
		case OP_LESS:		BINARY_COMP_OP(<); break;
		case OP_GREATER:	BINARY_COMP_OP(>); break;

		case OP_DEFINE_GLOBAL: {
			uint8_t IdIndex = CurrentChunk()->advance();
			std::string identifier = GetConstantStr(IdIndex);

			Value* v = pop();
			AddGlobal(identifier, v);
			break;
		}

		case OP_SET_GLOBAL: {
			uint8_t IdIndex = CurrentChunk()->advance();
			std::string identifier = GetConstantStr(IdIndex);

			Value* v = peek(0); // want to keep value on the stack in case the assignment is part of an expression

			if (globals[identifier] == nullptr) error(UNDEFINED_RAT, "Setting value to an undefined rat");

			globals[identifier] = v;
			break;
		}
						  
		case OP_GET_GLOBAL: {
			Value* var = FindGlobal();

			push(var);
			break;
		}

		case OP_GET_LOCAL: {
			uint8_t FrameSlot = CurrentChunk()->advance();
			Value* var = this->stack.stk[this->body->GetFrameStart() + FrameSlot + 1];

			push(var);
			break;
		}

		case OP_SET_LOCAL: {
			uint8_t FrameSlot = CurrentChunk()->advance();
			this->stack.stk[this->body->GetFrameStart() + FrameSlot + 1] = pop();

			break;
		}

		case OP_INC_GLOBAL: {
			Value* var = FindGlobal();

			if (var->GetType() != Value::NUM_T) {
				error(TYPE_ERROR, "Can't increment a non-number value");
			}
			NumValue* num = (NumValue*)var;
			num->SetValue(GetNumValue(num) + 1);
			
			push(num);
			break;
		}


		case OP_DEC_GLOBAL: {
			Value* var = FindGlobal();

			if (var->GetType() != Value::NUM_T) {
				error(TYPE_ERROR, "Can't decrement a non-number value");
			}
			NumValue* num = (NumValue*)var;
			num->SetValue(GetNumValue(num) - 1);
			push(num);

			break;
		}

		case OP_ADD_ASSIGN_GLOBAL: {
			switch (peek(0)->GetType()) {
				case Value::STRING_T: {
					StrValue* b = (StrValue*)pop();

					Value* va = FindGlobal();
					if (va->GetType() != Value::STRING_T) error(TYPE_ERROR, "Can only concatenate two strings");

					StrValue* a = (StrValue*)va;
					a->SetValue((std::string&)(*a + *b));
					push(a);
					break;
				}

				default: BINARY_ASSIGN_OP(+, true);
			}
			break;
		}

		case OP_SUB_ASSIGN_GLOBAL:			BINARY_ASSIGN_OP(-, false);	break;
		case OP_MULTIPLY_ASSIGN_GLOBAL:		BINARY_ASSIGN_OP(*, false); break;
		case OP_DIVIDE_ASSIGN_GLOBAL:		BINARY_ASSIGN_OP(/, false); break;

		case OP_BIT_AND_ASSIGN_GLOBAL:		BINARY_BIT_ASSIGN_OP(&);	break;
		case OP_BIT_OR_ASSIGN_GLOBAL:		BINARY_BIT_ASSIGN_OP(|);	break;
		case OP_BIT_XOR_ASSIGN_GLOBAL:		BINARY_BIT_ASSIGN_OP(^);	break;
		case OP_SHIFTL_ASSIGN_GLOBAL:		BINARY_BIT_ASSIGN_OP(<<);	break;
		case OP_SHIFTR_ASSIGN_GLOBAL:		BINARY_BIT_ASSIGN_OP(>>);	break;

		case OP_JUMP_IF_TRUE: {
			uint8_t JumpHighByte = this->CurrentChunk()->advance();
			uint8_t JumpLowByte = this->CurrentChunk()->advance();

			if (peek(0)->IsTruthy()) {
				short distance = (short)(JumpHighByte << 8) + (short)(JumpLowByte);
				this->CurrentChunk()->MoveIp(distance);
			}
			break;
		}
		case OP_JUMP_IF_FALSE: {
			uint8_t JumpHighByte = this->CurrentChunk()->advance();
			uint8_t JumpLowByte = this->CurrentChunk()->advance();

			if (!(peek(0)->IsTruthy())) {
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
			Value* v = peek(0);
			if (!IsIntegerValue(v)) error(TYPE_ERROR, "Can only use positive integer values as the operand to 'repeat'");
			int n = ((NumValue*)v)->GetValue();

			if (n <= 0)  error(TYPE_ERROR, "Can only use positive integer values as the operand to 'repeat'");
			break;
		}

		case OP_END_REPEAT: {
			Value *v = peek(0);
			if (!IsIntegerValue(v)) error(INTERNAL_ERROR, "");

			NumValue* nv = (NumValue*)v;
			v->SetValue(GetNumValue(v) - 1);

			int n = GetNumValue(v);

			if (n == 0) {
				pop();
				CurrentChunk()->MoveIp(4);  // skip over 'op_loop' instruction
			}

			break;
		}

		case OP_DEFINE_RUNNABLE: {
			uint8_t index = CurrentChunk()->advance();
			Value *v = CurrentChunk()->ReadConstant(index);
			if (v->GetType() != Value::RUNNABLE_T) error(INTERNAL_ERROR, "");
			
			RunnableValue* runnable = (RunnableValue *)v;
			AddGlobal(runnable->GetName(), runnable);
			break;
		}


		case OP_CALL: {
			Value* called = FindGlobal();

			switch (called->GetType())
			{
				case Value::RUNNABLE_T: {
					RunnableValue *runnable = ((RunnableValue*)called);
					uint8_t FrameIndex = this->stack.count - runnable->GetArity() - 1; // current capacity, minus arguments and identifier

					Chunk* c = new Chunk(runnable->GetChunk());
					RunnableValue* NewBody = new RunnableValue(runnable, c, this->body, FrameIndex);
					
					SetBody(NewBody);
					break;
				}

				default:
					error(TYPE_ERROR, "Can't call an object that isn't a runnable"); break;
				}

			break;
		}

		case OP_RETURN: {
			Value* ReturnVal = pop();

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


Value *Interpreter::pop() {
	if (this->stack.count <= 0) {
		error(STACK_UNDERFLOW, "Popping from empty stack");
	}

	stack.count--;
	Value *i = stack.stk[stack.count];

	return i;
}

void Interpreter::push(Value *value) {
	if (stack.count == StackSize) error(STACK_OVERFLOW, "Stack limit exceeded");
	stack.stk[stack.count++] = value;
	return;
}

Value *Interpreter::peek(int depth) {
	if (depth > stack.count - 1) error(STACK_UNDERFLOW, "Can't peek so deep into stack");
	return this->stack.stk[stack.count - 1 - depth];
}

float Interpreter::GetNumValue(Value* v) {
	return ((NumValue*)v)->GetValue();
}

bool Interpreter::GetBoolValue(Value* v) {
	return ((BoolValue*)v)->GetValue();
}

std::string Interpreter::GetStrValue(Value* v) {
	return ((StrValue*)v)->GetValue();
}

NumValue *Interpreter::NewObject(float f) {
	NumValue* res = new NumValue(f);
	res->next = this->objects;
	this->objects = res;
	return res;
}


BoolValue* Interpreter::NewObject(bool b) {
	BoolValue* res = new BoolValue(b);
	res->next = this->objects;
	this->objects = res;
	return res;
}

StrValue* Interpreter::NewObject(std::string& s) {
	StrValue* res = new StrValue(s);
	res->next = this->objects;
	this->objects = res;
	return res;
}

Value* Interpreter::NewObject(nullptr_t p) {
	Value* res = new Value(); // 'None' type
	res->next = this->objects;
	this->objects = res;
	return res;
}

std::string Interpreter::GetConstantStr(uint8_t index) {
	Value* v = CurrentChunk()->ReadConstant(index);
	return ((StrValue*)v)->GetValue();
}

bool Interpreter::GetConstantBool(uint8_t index) {
	Value* v = CurrentChunk()->ReadConstant(index);
	return ((BoolValue*)v)->GetValue();
}

float Interpreter::GetConstantNum(uint8_t index) {
	Value* v = CurrentChunk()->ReadConstant(index);
	return ((NumValue*)v)->GetValue();
}



Value* Interpreter::FindGlobal() {
	uint8_t IdIndex = CurrentChunk()->advance();
	std::string identifier = GetConstantStr(IdIndex);
	Value* var = this->globals[identifier];

	if (var == nullptr) error(UNDEFINED_RAT, "Undefined rat " + identifier);

	return var;
}

void Interpreter::AddGlobal(std::string& name, Value* value) {
	this->globals.insert({ name, value });
}

std::string Interpreter::TraceStack(int CodeOffset) {
	std::string trace = "";
	for (uint8_t i = 0; i < this->stack.count; i++) {
		trace += ("[ " + this->stack.stk[i]->ToString() + "]\t");
	}
	trace += '\n';
	return (std::to_string(CodeOffset) + "\t" + trace);
}

void Interpreter::error(ExitCode e, std::string msg) {
	int line = CurrentChunk()->CountLines();
	std::cerr << "[Runtime error in line " << line << "]: " << msg << "\n";
	throw e;
}