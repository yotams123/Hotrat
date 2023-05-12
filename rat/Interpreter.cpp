#include "Interpreter.h"

Interpreter::Interpreter(Chunk* chunk) {
	this->chunk = chunk;
	
	stack.count = 0;
}

Interpreter::~Interpreter() {}

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

	uint8_t op = chunk->advance();
	switch (op)
	{
	case OP_NEWLINE: {
		if (stack.count != 0) std::cout << pop() << "\n\n\n";
		break; 
	} 
		case OP_CONSTANT:	push(chunk->ReadConstant(chunk->advance())); break;
	
		case OP_NEGATE: {
			int a = pop();
			push(-a);
			break;
		}
		case OP_NOT: {
			int a = pop();
			push(~a);
			break;
		}

		case OP_ADD:{
			int b = pop();
			int a = pop();
			push(a + b);
			break;
		}
		case OP_SUB: {
			int b = pop();
			int a = pop();
			push(a - b);
			break;
		}
		case OP_MULTIPLY: {
			int b = pop();
			int a = pop();
			push(a * b);
			break;
		}
		case OP_DIVIDE: {
			int b = pop();
			int a = pop();
			push(a / b);
			break;
		}

		case OP_BIT_AND: {
			int b = pop();
			int a = pop();
			push(a & b);
			break;
		}
		case OP_BIT_OR: {
			int b = pop();
			int a = pop();
			push(a | b);
			break;
		}
		case OP_BIT_XOR: {
			int b = pop();
			int a = pop();
			push(a ^ b);
			break;
		}

		case OP_SHIFT_LEFT: {
			int b = pop();
			int a = pop();
			push(a << b);
			break;
		}
		case OP_SHIFT_RIGHT: {
			int b = pop();
			int a = pop();
			push(a >> b);
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

int Interpreter::pop() {
	if (stack.count == 0) {
		error(EMPTY_STACK, "Popping from empty stack");
	}
	int i = stack.stk[--stack.count];

	return i;
}

void Interpreter::push(int value) {
	if (stack.count == StackSize) throw STACK_OVERFLOW;
	stack.stk[stack.count++] = value;
	return;
}

std::string Interpreter::TraceStack(int CodeOffset) {
	std::string trace = "";
	for (uint8_t i = 0; i < stack.count; i++) {
		trace += ("[ " + std::to_string(stack.stk[i]) + "]\t");
	}
	trace += '\n';
	return (std::to_string(CodeOffset) + "\t" + trace);
}

void Interpreter::error(int e, std::string msg) {
	int line = chunk->CountLines();
	std::cerr << "[Runtime error in line " << line << "]: " << msg << "\n";
	throw -e;
}