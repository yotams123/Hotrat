#pragma once

#include <iostream>
#include <vector>

#include "Token.h"
#include "Value.h"

typedef enum {
	OP_NEWLINE,
	OP_RETURN,

	OP_CONSTANT,

	OP_TRUE,
	OP_FALSE,

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

	OP_EQUALS,
	OP_GREATER,
	OP_LESS,

	OP_DEFINE_GLOBAL,
	OP_DECLARE_GLOBAL_BOOL,
	OP_DECLARE_GLOBAL_NUM,
	OP_DECLARE_GLOBAL_STR,

	OP_SET_GLOBAL,
	OP_GET_GLOBAL,

	OP_NEGATE,
} Opcode;

typedef struct Chunk {
private:

	std::vector<uint8_t> code;
	std::vector<uint8_t>::iterator ip;

	std::vector<Value *> constants;
	struct Chunk* enclosing;

public:
	Chunk();
	~Chunk();

	uint8_t AddConstant(Token);

	void Append(uint8_t);
	void Append(uint8_t, uint8_t);

	uint8_t advance();
	Value *ReadConstant(uint8_t index);

	std::vector<uint8_t>& GetCode();
	bool IsAtEnd();

	_int64 GetOffset();
	void SyncIP();
	int CountLines();

} Chunk;

