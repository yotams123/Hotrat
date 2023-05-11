#include "Interpreter.h"

Interpreter::Interpreter(Chunk* chunk) {
	this->chunk = chunk;
	temps = std::stack<int>();
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
}

void Interpreter::RunCommand() {
	uint8_t op = chunk->advance();

	switch (op)
	{
	case OP_CONSTANT:	push(chunk->ReadConstant(chunk->advance())); break;
	
	case OP_NEGATE: {
		int a = pop();
		push(-a);
	}
	case OP_NOT: {
		int a = pop();
		push(~a);
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
		std::cerr << "Unrecognized opcode " << op << "\n"; // TODO
		break;
	}
	std::cout << temps.top() << "\n";
}

int Interpreter::pop() {
	if (temps.empty()) {
		std::cerr << "Popping from stack when there is nothing to pop";
		throw EMPTY_STACK;
	}
	int i = temps.top();
	temps.pop();

	return i;
}

void Interpreter::push(int value) {
	temps.push(value);
	return;
}