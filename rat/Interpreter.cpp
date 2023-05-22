#include "Interpreter.h"

bool IsIntegerValue(Value *f) {
	if (f->GetType() != Value::NUM_T) return false;
	float n = ((NumValue*)f)->GetValue();
	if (n / 1 == n) return true;
	return false;
}

Interpreter::Interpreter(Chunk* chunk) {
	this->chunk = chunk;
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
	while (!(chunk->IsAtEnd())) {
		try {
			RunCommand();
		}
		catch (int e) {
			return e;
		}
	}

	return 0;
}

void Interpreter::RunCommand() {
#ifdef DEBUG_TRACE_STACK
	int offset = chunk->GetOffset();
#endif // DEBUG_TRACE_STACK

#define BINARY_NUM_OP(op)  {\
	Value *b = pop(); \
	Value *a = pop(); \
	if (b->GetType() == Value::NUM_T && a->GetType() == Value::NUM_T){\
		\
		float n1 = ((NumValue *)a)->GetValue(); \
		float n2 = ((NumValue *)b)->GetValue(); \
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
		float n1 = ((NumValue *)a)->GetValue(); \
		float n2 = ((NumValue *)b)->GetValue(); \
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
		int n1 = ((NumValue *)a)->GetValue(); \
		int n2 = ((NumValue *)b)->GetValue(); \
		push(NewObject((float)(n1 op n2))); \
	} else {\
		error(TYPE_ERROR, "Can't perform this operation on a non-integer");\
	}\
}

	uint8_t op = chunk->advance();
	switch (op)
	{
		case OP_NEWLINE: {
			if (stack.count != 0) std::cout << pop()->ToString() << "\n\n\n";
			break; 
		} 
		case OP_CONSTANT:	push(chunk->ReadConstant(chunk->advance())); break;
		

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
				float n = ((NumValue*)a)->GetValue();
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
					float n = ((NumValue*)a)->GetValue();
					if (n / 1 == n)	push(new NumValue(~(int)n));
					else error(TYPE_ERROR, "Can't perform bitwise operation on a non-integer");
					break;
				}
				case Value::BOOL_T: {
					bool b = ((BoolValue*)a)->GetValue();
					push(NewObject(!b));
					break;
				}
			}
			break;
		}

		case OP_ADD:			BINARY_NUM_OP(+);	break;
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
			uint8_t IdIndex = chunk->advance();
			std::string identifier = GetConstantStr(IdIndex);

			Value* v = pop();
			AddGlobal(identifier, v);
			break;
		}

		case OP_SET_GLOBAL: {
			uint8_t IdIndex = chunk->advance();
			std::string identifier = GetConstantStr(IdIndex);

			Value* v = pop();
			globals[identifier] = v;
			break;
		}
						  
		case OP_GET_GLOBAL: {
			uint8_t IdIndex = chunk->advance();
			std::string identifier = GetConstantStr(IdIndex);

			push(globals[identifier]);
			break;
		}

		default:
			error(UNRECOGNIZED_OPCODE, "Unrecognized opcode " + op);
			break;
	}

#ifdef DEBUG_TRACE_STACK
	std::cout << TraceStack(offset);
#endif
}

Value *Interpreter::pop() {
	if (stack.count <= 0) {
		error(STACK_UNDERFLOW, "Popping from empty stack");
	}

	Value *i = stack.stk[--stack.count];

	return i;
}

void Interpreter::push(Value *value) {
	if (stack.count == StackSize) throw STACK_OVERFLOW;
	stack.stk[stack.count++] = value;
	return;
}

Value *Interpreter::peek(int depth) {
	if (depth > stack.count - 1) error(STACK_UNDERFLOW, "Can't peek so deep into stack");
	return this->stack.stk[stack.count - 1 - depth];
}

NumValue *Interpreter::NewObject(float f) {
	NumValue* res = new NumValue(f);
	res->next = objects;
	objects = res;
	return res;
}

BoolValue* Interpreter::NewObject(bool b) {
	BoolValue* res = new BoolValue(b);
	res->next = objects;
	objects = res;
	return res;
}

StrValue* Interpreter::NewObject(std::string& s) {
	StrValue* res = new StrValue(s);
	res->next = objects;
	objects = res;
	return res;
}

Value* Interpreter::NewObject(nullptr_t p) {
	Value* res = new Value(); // 'None' type
	res->next = objects;
	objects = res;
	return res;
}



std::string Interpreter::GetConstantStr(uint8_t index) {
	Value* v = chunk->ReadConstant(index);
	return ((StrValue*)v)->GetValue();
}



void Interpreter::AddGlobal(std::string& name, Value* value) {
	this->globals.insert({ name, value });
}

std::string Interpreter::TraceStack(int CodeOffset) {
	std::string trace = "";
	for (uint8_t i = 0; i < stack.count; i++) {
		trace += ("[ " + stack.stk[i]->ToString() + "]\t");
	}
	trace += '\n';
	return (std::to_string(CodeOffset) + "\t" + trace);
}

void Interpreter::error(int e, std::string msg) {
	int line = chunk->CountLines();
	std::cerr << "[Runtime error in line " << line << "]: " << msg << "\n";
	throw -e;
}