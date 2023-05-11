#pragma once

#include "Token.h"
#include "Interpreter.h"
#include "Chunk.h"

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


class Compiler
{

public:
	Compiler(std::vector<Token>&);
	~Compiler();

	Chunk* Compile();

private:
	std::vector<Token> tokens;
	std::vector<Token>::iterator CurrentToken;
	bool HadError;

	int COMPILE_ERROR = 45; // compile error code

	typedef void (Compiler::* ParseFunction) ();
	
	typedef struct {
		ParseFunction prefix;
		ParseFunction infix;
		Precedence precedence;
	} ParseRule;

	ParseRule RuleTable[NumTokenTypes];
	

	Chunk* CurrentChunk;

	void error(std::string msg);
	void synchronize();
	int CountLines();

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

