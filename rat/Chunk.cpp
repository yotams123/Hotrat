#include "Chunk.h"

Chunk::Chunk() {
	ip = 0;

	constants = std::vector<Value*>();
	enclosing = nullptr;
}

Chunk::~Chunk() {
	std::vector<Value*>::iterator i;
	for (i = constants.begin(); i < constants.end(); i++)
	{
		delete* i;
	}
}

uint8_t Chunk::advance() {
	return code[ip++];
}


uint8_t Chunk::AddConstant(Token constant) {
	if (constants.size() >= 256) return -1; // TODO throw error

	Value *val;

	switch (constant.GetType()) {
		case NUM_LITERAL:	val = new NumValue(std::stof(constant.GetLexeme()));	break;
		case TRUE:			val = new BoolValue(true);
		case FALSE:			val = new BoolValue(false);

		case STRING_LITERAL:
		case IDENTIFIER:	val = new StrValue((std::string&)constant.GetLexeme());
	}

	constants.push_back(val);
	return (uint8_t)constants.size() - 1; // index of constant
}

Value *Chunk::ReadConstant(uint8_t index) {
	return constants[index];
}


void Chunk::Append(uint8_t byte) {
	code.push_back(byte);
}

void Chunk::Append(uint8_t byte1, uint8_t byte2) {
	code.push_back(byte1);
	code.push_back(byte2);
}

std::vector<uint8_t>& Chunk::GetCode() {
	return this->code;
}

bool Chunk::IsAtEnd() {
	return this->ip > (this->code.size() - 1);
}

int Chunk::CountLines() {
	int line = 1;
	short op = 0;
	while (op < ip) {
		switch (this->code[op]) {
			case OP_CONSTANT:
			case OP_DEFINE_GLOBAL:
			case OP_GET_GLOBAL:
			case OP_SET_GLOBAL:

			case OP_INC:
			case OP_DEC: {
				op++;
				op++;  // skip over opcode and operand
				break;
			}
			
			case OP_NEWLINE:	line++;
			default:	op++;
		}
	}
	return line;
}

short Chunk::GetOffset() {
	return this->ip;
}

short Chunk::GetSize() {
	return this->code.size();
}


void Chunk::MoveIp(short distance) {
	ip += distance;
}

void Chunk::PatchJump(short JumpIndex, short distance) {
	this->code[JumpIndex - 1] = (uint8_t)((distance >> 8) & 0xFF);
	this->code[JumpIndex] = (uint8_t)(distance & 0xFF);
}