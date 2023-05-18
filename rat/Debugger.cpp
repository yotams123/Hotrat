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
	line = 1;
	PrintLineNum = "1";

	std::cout << "==" << ChunkName << "==\n";

	while (offset < code.size()) {
		DisassembleInstruction();
	}
	std::cout << "\n\n\n";
}

void Debugger::DisassembleInstruction() {
	std::cout << std::setw(4) << std::right << offset << "\t" << PrintLineNum << "\t";
	PrintLineNum = "|";

	Opcode instruction = (Opcode)code[offset];

	switch (instruction) {
		
		case OP_NEWLINE: {
			PrintLineNum = std::to_string(++line);
			SimpleOperation("OP_NEWLINE");
			break;
		}
		
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

		case OP_TRUE:	SimpleOperation("OP_TRUE");		break;
		case OP_FALSE:	SimpleOperation("OP_FALSE");	break;

		case OP_EQUALS:		SimpleOperation("OP_EQUALS");	break;
		case OP_LESS:		SimpleOperation("OP_LESS");		break;
		case OP_GREATER:	SimpleOperation("OP_GREATER");	break;

		case OP_NEGATE: SimpleOperation("OP_NEGATE");	break;
		case OP_NOT:	SimpleOperation("OP_NOT");		break;

		case OP_DECLARE_GLOBAL_BOOL:	ConstantOperation("OP_DECLARE_GLOBAL_BOOL");	break;
		case OP_DECLARE_GLOBAL_NUM:		ConstantOperation("OP_DECLARE_GLOBAL_NUM");		break;
		case OP_DECLARE_GLOBAL_STR:		ConstantOperation("OP_DECLARE_GLOBAL_STR");		break;

		case OP_DEFINE_GLOBAL:		ConstantOperation("OP_DEFINE_GLOBAL");	break;
		case OP_SET_GLOBAL:			ConstantOperation("OP_SET_GLOBAL");		break;
		case OP_GET_GLOBAL:			ConstantOperation("OP_GET_GLOBAL");		break;


		default:	std::cout << "Unrecognized instruction" << instruction << "\n";
	}
}