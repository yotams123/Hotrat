#include "Interpreter.h"

Interpreter::Interpreter(Chunk* chunk) {
	this->chunk = chunk;
	
	stack.count = 0;
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
	float n2 = b->GetValue().n; \
	\
	Value *a = pop(); \
	float n1 = a->GetValue().n; \
	\
	push(NewObject(n1 op n2));\
}

#define BINARY_BIT_OP(op) {\
	Value *b = pop();\
	float n2 = b->GetValue().n; \
	if (n2 != (int)n2) error(TYPE_ERROR, "Can't perform bitwise operation on a non-integer"); \
	\
	Value* a = pop(); \
	float n1 = a->GetValue().n; \
	if (n1 != (int)n1) error(TYPE_ERROR, "Can't perform bitwise operation on a non-integer"); \
	\
	push(NewObject((float)((int) n1 op (int)n2)));\
}

	uint8_t op = chunk->advance();
	switch (op)
	{
		case OP_NEWLINE: {
			/*if (stack.count != 0) std::cout << pop() << "\n\n\n";*/
			pop();
			break; 
		} 
		case OP_CONSTANT:	push(chunk->ReadConstant(chunk->advance())); break;
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

			float n = a->GetValue().n;
			push(new NumValue(-n));
			break;
		}

		case OP_NOT: {
			Value *a = pop();
			Value::datatype t = a->GetType();
			switch (t) {
				case Value::NUM_T: {
					float n = a->GetValue().n;
					if (n == (int)n)	push(new NumValue(~(int)n));
					else error(TYPE_ERROR, "Can't perform bitwise operation on a non-integer");
					break;
				}
				case Value::BOOL_T: {
					bool b = a->GetValue().b;
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
		error(EMPTY_STACK, "Popping from empty stack");
	}

	Value *i = stack.stk[--stack.count];

	return i;
}

void Interpreter::push(Value *value) {
	if (stack.count == StackSize) throw STACK_OVERFLOW;
	stack.stk[stack.count++] = value;
	return;
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

std::string Interpreter::TraceStack(int CodeOffset) {
	std::string trace = "";
	for (uint8_t i = 0; i < stack.count; i++) {
		/*trace += ("[ " + std::to_string(stack.stk[i]) + "]\t");*/
	}
	trace += '\n';
	return (std::to_string(CodeOffset) + "\t" + trace);
}

void Interpreter::error(int e, std::string msg) {
	int line = chunk->CountLines();
	std::cerr << "[Runtime error in line " << line << "]: " << msg << "\n";
	throw -e;
}