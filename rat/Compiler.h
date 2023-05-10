#pragma once

#include "Token.h"
#include "Interpreter.h"

#include <vector>
#include <iostream>

typedef enum {
	PREC_END,
	PREC_NONE,
	PREC_ASSIGN,
	PREC_TERM,
	PREC_FACTOR,
	PREC_BIT,
	PREC_UNARY,
	PREC_LITERAL
} Precedence;

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

	std::vector<uint8_t>& GetCode();
} Chunk;


class Compiler
{

public:
	Compiler(std::vector<Token>&);
	~Compiler();

	Chunk* Compile();

private:
	std::vector<Token> tokens;
	std::vector<Token>::iterator CurrentToken;

	typedef void (Compiler::* ParseFunction) ();
	
	typedef struct {
		ParseFunction prefix;
		ParseFunction infix;
		Precedence precedence;
	} ParseRule;

	ParseRule RuleTable[NUM_TOKENTYPES];
	

	Chunk* CurrentChunk;

	// parsing
	void literal();
	void unary();
	void binary();
	void grouping();
	void expression();

	ParseRule& GetRule(TokenType);
	void ParsePrecedence(Precedence);

	Token advance();
	bool match(TokenType type);
	void consume(TokenType type, std::string ErrorMsg);

	// bytecode
	void EmitByte(uint8_t byte);
	void EmitBytes(uint8_t byte1, uint8_t byte2);
	void EmitReturn();
};

