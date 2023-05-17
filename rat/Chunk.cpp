#include "Chunk.h"

Chunk::Chunk() {
	ip = code.begin();

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
	return *(ip++);
}


uint8_t Chunk::AddConstant(Token constant) {
	if (constants.size() >= 256) return -1; // TODO throw error

	Value *val;

	switch (constant.GetType()) {
		case INT_LITERAL:	val = new NumValue(std::stoi(constant.GetLexeme()));	break;
		case FLOAT_LITERAL:	val = new NumValue(std::stof(constant.GetLexeme()));	break;

		case TRUE:			val = new BoolValue(true);
		case FALSE:			val = new BoolValue(false);
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
	return code;
}

bool Chunk::IsAtEnd() {
	return ip == code.end();
}

void Chunk::SyncIP() {
	ip = code.begin();
}

int Chunk::CountLines() {
	int line = 1;
	std::vector<uint8_t>::iterator op = code.begin();
	for (; op < ip; op++) {
		if (*op == OP_NEWLINE) line++;
	}
	return line;
}

_int64 Chunk::GetOffset() {
	return std::distance(code.begin(), ip);
}