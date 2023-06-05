#pragma once

#include <iostream>
#include <vector>

#include "Token.h"
#include "Value.h"

typedef enum {
	OP_NEWLINE,

	OP_CONSTANT,
	OP_POP,

	OP_NONE,
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

	OP_SET_GLOBAL,
	OP_GET_GLOBAL,

	OP_INC,
	OP_DEC,

	OP_ADD_ASSIGN,
	OP_SUB_ASSIGN,
	OP_MULTIPLY_ASSIGN,
	OP_DIVIDE_ASSIGN,

	OP_BIT_AND_ASSIGN,
	OP_BIT_OR_ASSIGN,
	OP_BIT_XOR_ASSIGN,
	OP_SHIFTL_ASSIGN,
	OP_SHIFTR_ASSIGN,

	OP_JUMP,
	OP_JUMP_IF_TRUE,
	OP_JUMP_IF_FALSE,
	OP_LOOP,

	OP_REPEAT, 
	OP_END_REPEAT,

	OP_DEFINE_RUNNABLE,
	OP_CALL,
	OP_RETURN,

	OP_NEGATE,
} Opcode;

typedef struct Chunk {
private:

	std::vector<uint8_t> code;
	short ip;

	std::vector<Value *> constants;
	struct Chunk* enclosing;

public:
	Chunk();
	Chunk(Chunk* enclosing);
	Chunk(Chunk *code, Chunk* enclosing);
	~Chunk();

	uint8_t AddConstant(Token);
	uint8_t AddConstant(Chunk *ByteCode, uint8_t arity, std::string name);	// to add runnables
	void ClearConstants();

	void Append(uint8_t);
	void Append(uint8_t, uint8_t);

	uint8_t advance();
	Value *ReadConstant(uint8_t index);

	std::vector<uint8_t>& GetCode();
	std::vector<Value*>& GetConstants();
	bool IsAtEnd();

	short GetOffset();
	short GetSize();
	Chunk* GetEnclosing();
	void MoveIp(short distance);

	void PatchJump(short JumpIndex, short distance);

	int CountLines();
} Chunk;

