#pragma once

#include <iostream>
#include <vector>

#include "Token.h"

typedef enum {
	OP_NEWLINE,
	OP_RETURN,

	OP_CONSTANT,

	OP_TRUE,
	OP_FALSE,
	OP_NONE,

	OP_ADD,
	OP_SUB,
	OP_DIVIDE,
	OP_MULTIPLY,

	OP_SHIFT_LEFT,
	OP_SHIFT_RIGHT,

	OP_BIT_AND,
	OP_BIT_OR,
	OP_BIT_XOR,
	OP_NOT,

	OP_NEGATE,
} Opcode;

typedef struct Chunk {
private:

	std::vector<uint8_t> code;
	std::vector<uint8_t>::iterator ip;

	std::vector<int> constants;
	struct Chunk* enclosing;

public:
	Chunk();
	uint8_t AddConstant(Token);

	void Append(uint8_t);
	void Append(uint8_t, uint8_t);

	uint8_t advance();
	int ReadConstant(uint8_t index);

	std::vector<uint8_t>& GetCode();
	bool IsAtEnd();

	void SyncIP();
	int CountLines();
} Chunk;
