#include "Chunk.h"

Chunk::Chunk() {
	ip = code.begin();

	constants = std::vector<int>();
	enclosing = nullptr;
}

uint8_t Chunk::advance() {
	return *(ip++);
}


uint8_t Chunk::AddConstant(Token constant) {
	if (constants.size() >= 256) return -1; // TODO throw error

	int value = stoi(constant.GetLexeme());
	constants.push_back(value);

	return (uint8_t)constants.size() - 1; // index of constant
}

int Chunk::ReadConstant(uint8_t index) {
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