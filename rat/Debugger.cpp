#include "Debugger.h"

Debugger::Debugger(Chunk *chunk, std::string name){
	this->chunk = chunk;
	
	ChunkName = name;
	code = chunk->GetCode();

	offset = 0;
}

Debugger::~Debugger() {
}

void Debugger::ConstantOperation(const std::string& name) {
	uint8_t constant = code[offset + 1];

	std::cout << std::setw(16) << std::left << name << std::setw(4) << std::left << std::to_string(constant) << "\n";

	offset += 2;
}

void Debugger::SimpleOperation(const std::string& name) {
	std::cout << std::setw(16) << std::left << name << "\n";
	offset++;
}

void Debugger::DisassembleChunk() {
	std::cout << "==" << ChunkName << "==\n";

	while (offset < code.size()) {
		DisassembleInstruction();
	}
}

void Debugger::DisassembleInstruction() {
	std::cout << std::setw(4) << std::right << offset << "\t";

	uint8_t instruction = code[offset];

	switch (instruction) {
		case OP_CONSTANT:	ConstantOperation("OP_CONSTANT");	break;
		case OP_RETURN:		SimpleOperation("OP_RETURN");		break;
	
		case OP_ADD:		SimpleOperation("OP_ADD");		break;
		case OP_SUB:		SimpleOperation("OP_SUB");		break;
		case OP_MULTIPLY:	SimpleOperation("OP_MULTIPLY"); break;
		case OP_DIVIDE:		SimpleOperation("OP_DIVIDE");	break;

		case OP_SHIFT_LEFT:		SimpleOperation("OP_SHIFT_LEFT");	break;
		case OP_SHIFT_RIGHT:	SimpleOperation("OP_SHIFT_RIGHT");	break;

		case OP_BIT_AND:		SimpleOperation("OP_BIT_AND");	break;
		case OP_BIT_OR:			SimpleOperation("OP_BIT_OR");	break;
		case OP_BIT_XOR:		SimpleOperation("OP_BIT_XOR");	break;

		case OP_NONE:	SimpleOperation("OP_NONE");		break;
		case OP_TRUE:	SimpleOperation("OP_TRUE");		break;
		case OP_FALSE:	SimpleOperation("OP_FALSE");	break;

		case OP_NEGATE: SimpleOperation("OP_NEGATE");	break;
		case OP_NOT:	SimpleOperation("OP_NOT");		break;

		default:
			std::cout << "Unrecognized instruction\n";
	}
}