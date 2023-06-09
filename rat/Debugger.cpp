#include "Debugger.h"

#define OPCODE_NAME_LEN 32

Debugger::Debugger(Chunk *chunk, std::string name){
	this->chunk = chunk;

	ChunkName = name;
	code = chunk->GetCode();

	offset = 0;
}

Debugger::~Debugger() {
}

void Debugger::ConstantOperation(const std::string& name) {
	// Print an opcode with one operand, an index in the chunk's constants table
	uint8_t constant = code[offset + 1];

	std::cout << std::setw(OPCODE_NAME_LEN) << std::left << name << std::setw(4) << std::left << std::to_string(constant) << "\n";

	offset += 2;
}

void Debugger::SimpleOperation(const std::string& name) {
	// Print a opcode with no operands
	std::cout << std::setw(OPCODE_NAME_LEN) << std::left << name << "\t\n";
	offset++;
}

void Debugger::JumpOperation(const std::string& name) {
	// Print a jump opcode. Highlight the start and end points of the jump.
	short distance = (short)((code[offset + 1] << 8));
	distance += (short)(code[offset + 2] & 0xFF);

	if (name == "OP_LOOP") distance *= -1;
	std::cout << std::setw(OPCODE_NAME_LEN) << std::left << name << std::setw(4) << std::left << 
		std::to_string(offset + 2) << "--> " << std::to_string(offset + 3 + distance) <<"\n";

	offset += 3;
}


void Debugger::CallNativeOperation(const std::string& name) {
	// Print the opcode to call a native runnable. The opcode has special operands.

	uint8_t index = code[offset + 1];
	uint8_t arity = code[offset + 2];

	std::cout << std::setw(OPCODE_NAME_LEN) << std::left << name << std::setw(4) << std::left <<
		std::to_string(index) << " arity = " << std::to_string(arity) << "\n";

	offset += 3;
}


void Debugger::RunnableDefinition(const std::string& name) {
	// Print the opcode to define a user-defined runnable. The opcode has special operands.

	std::string& rname = chunk->ReadConstant(code[offset + 1]).GetObjectValue()->ToString();
	uint8_t lines = code[offset + 2];

	std::cout << std::setw(OPCODE_NAME_LEN) << std::left << name << std::setw(4) << std::left <<
		rname << " \t\tlinecount = " << std::to_string(lines) << "\n";

	this->runnables.push_back({code[offset + 1], line + 1});

	this->line += lines;
	offset += 3;
}

void Debugger::DisassembleScript() {
	line = 1;
	PrintLineNum = "1";

	std::cout << "==" << ChunkName << "==\n";

	while (offset < code.size()) {
		DisassembleInstruction();
	}

	Chunk* Script = this->chunk;

	int rsize = runnables.size();
	for (int i = 0; i < rsize; i++) {
		int index = std::get<0>(runnables[i]);
		int LineOffset = std::get<1>(runnables[i]);
		RunnableValue* rv = (RunnableValue *)(chunk->ReadConstant(index).GetObjectValue());
		DisassembleRunnable(rv, LineOffset);
		this->chunk = Script;
	}
	std::cout << "\n\n\n";
}


void Debugger::DisassembleRunnable(RunnableValue *runnable, int LineOffset) {
	line = LineOffset;
	PrintLineNum = std::to_string(LineOffset);

	this->ChunkName = runnable->GetName();
	std::cout << "\n\n\n==" << ChunkName << "==\n";

	this->offset = 0;
	this->chunk = runnable->GetChunk();
	this->code = this->chunk->GetCode();

	int ChunkSize = this->chunk->GetSize();
	while (this->offset < ChunkSize) {
		DisassembleInstruction();
	}

	std::cout << "\n\n\n";
}

void Debugger::DisassembleInstruction() {
	std::cout << std::setw(4) << std::right << offset << "\t" << PrintLineNum << "\t";
	PrintLineNum = "|";

	Opcode instruction = (Opcode)(code[offset]);

	switch (instruction) {
		
		case OP_NEWLINE: {
			PrintLineNum = std::to_string(++line);
			SimpleOperation("OP_NEWLINE");
			break;
		}

		case OP_POP:		SimpleOperation("OP_POP");		break;

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

		case OP_EQUALS:		SimpleOperation("OP_EQUALS");	break;
		case OP_LESS:		SimpleOperation("OP_LESS");		break;
		case OP_GREATER:	SimpleOperation("OP_GREATER");	break;

		case OP_NEGATE: SimpleOperation("OP_NEGATE");	break;
		case OP_NOT:	SimpleOperation("OP_NOT");		break;

		case OP_DEFINE_GLOBAL:		ConstantOperation("OP_DEFINE_GLOBAL");	break;
		case OP_SET_GLOBAL:			ConstantOperation("OP_SET_GLOBAL");		break;
		case OP_GET_GLOBAL:			ConstantOperation("OP_GET_GLOBAL");		break;
		case OP_CONSTANT:			ConstantOperation("OP_CONSTANT");		break;

		case OP_GET_LOCAL:			ConstantOperation("OP_GET_LOCAL");		break;
		case OP_SET_LOCAL:			ConstantOperation("OP_SET_LOCAL");		break;

		case OP_INC_GLOBAL:				ConstantOperation("OP_INC_GLOBAL");			break;
		case OP_DEC_GLOBAL:				ConstantOperation("OP_DEC_GLOBAL");			break;

		case OP_INC_LOCAL:				ConstantOperation("OP_INC_LOCAL");			break;
		case OP_DEC_LOCAL:				ConstantOperation("OP_DEC_LOCAL");			break;

		case OP_ADD_ASSIGN_GLOBAL:			ConstantOperation("OP_ADD_ASSIGN_GLOBAL");			break;
		case OP_SUB_ASSIGN_GLOBAL:			ConstantOperation("OP_SUB_ASSIGN_GLOBAL");			break;
		case OP_MULTIPLY_ASSIGN_GLOBAL:		ConstantOperation("OP_MULTIPLY_ASSIGN_GLOBAL");		break;
		case OP_DIVIDE_ASSIGN_GLOBAL:		ConstantOperation("OP_DIVIDE_ASSIGN_GLOBAL");		break;

		case OP_ADD_ASSIGN_LOCAL:			ConstantOperation("OP_ADD_ASSIGN_LOCAL");			break;
		case OP_SUB_ASSIGN_LOCAL:			ConstantOperation("OP_SUB_ASSIGN_LOCAL");			break;
		case OP_MULTIPLY_ASSIGN_LOCAL:		ConstantOperation("OP_MULTIPLY_ASSIGN_LOCAL");		break;
		case OP_DIVIDE_ASSIGN_LOCAL:		ConstantOperation("OP_DIVIDE_ASSIGN_LOCAL");		break;

		case OP_BIT_AND_ASSIGN_GLOBAL:		ConstantOperation("OP_BIT_AND_ASSIGN_GLOBAL");		break;
		case OP_BIT_OR_ASSIGN_GLOBAL:		ConstantOperation("OP_BIT_OR_ASSIGN_GLOBAL");		break;
		case OP_BIT_XOR_ASSIGN_GLOBAL:		ConstantOperation("OP_BIT_XOR_ASSIGN_GLOBAL");		break;
		case OP_SHIFTL_ASSIGN_GLOBAL:		ConstantOperation("OP_SHIFTL_ASSIGN_GLOBAL");		break;
		case OP_SHIFTR_ASSIGN_GLOBAL:		ConstantOperation("OP_SHIFTR_ASSIGN_GLOBAL");		break;

		case OP_BIT_AND_ASSIGN_LOCAL:		ConstantOperation("OP_BIT_AND_ASSIGN_LOCAL");		break;
		case OP_BIT_OR_ASSIGN_LOCAL:		ConstantOperation("OP_BIT_OR_ASSIGN_LOCAL");		break;
		case OP_BIT_XOR_ASSIGN_LOCAL:		ConstantOperation("OP_BIT_XOR_ASSIGN_LOCAL");		break;
		case OP_SHIFTL_ASSIGN_LOCAL:		ConstantOperation("OP_SHIFTL_ASSIGN_LOCAL");		break;
		case OP_SHIFTR_ASSIGN_LOCAL:		ConstantOperation("OP_SHIFTR_ASSIGN_LOCAL");		break;

		case OP_JUMP:				JumpOperation("OP_JUMP");			break;
		case OP_JUMP_IF_TRUE:		JumpOperation("OP_JUMP_IF_TRUE");	break;
		case OP_JUMP_IF_FALSE:		JumpOperation("OP_JUMP_IF_FALSE");	break;
		case OP_LOOP:				JumpOperation("OP_LOOP");			break;

		case OP_REPEAT:				SimpleOperation("OP_REPEAT");		break;
		case OP_END_REPEAT:			SimpleOperation("OP_END_REPEAT");	break;


		case OP_DEFINE_RUNNABLE:	RunnableDefinition("OP_DEFINE_RUNNABLE");	break;
		case OP_CALL:				ConstantOperation("OP_CALL");				break;
		case OP_RETURN:				SimpleOperation("OP_RETURN");				break;

		case OP_CALL_NATIVE:		CallNativeOperation("OP_CALL_NATIVE");				break;

		default: {
			std::cout << "Unrecognized instruction" << instruction << "\t\n";
			offset++;
		}
	}
}