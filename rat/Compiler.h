#pragma once

#include "Token.h"
#include "Interpreter.h"
#include "Chunk.h"
#include "Value.h"

#include <vector>
#include <iostream>

typedef enum {
	PREC_END,
	PREC_NONE,
	PREC_ASSIGN,
	PREC_AND,
	PREC_OR,
	PREC_COMPARE,
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

	RunnableValue* Compile();

private:
	std::vector<Token> tokens;
	int CurrentTokenOffset;
	bool HadError;

	static enum {
		COMPILE_OK = 0,

		UNRECOGNIZED_TOKEN = 101,
		UNEXPECTED_TOKEN,
		INTERNAL_ERROR,  // Errors that have to do with the compiler, not the user.
		UNCLOSED_BLOCK,
		FLOAT_OVERFLOW,  // errors that are common with chunk
		TABLE_OVERFLOW,
		BLOCKED_RUNNABLE,
		UNDEFINED_RUNNABLE,

		BREAK_IF = 150,
		BREAK_WHILE,
		BREAK_FOR,
		BREAK_REPEAT,
		BREAK_RUNNABLE,
		BREAK_RAT
	} ExitCode;// compile error code


	static enum ChunkType {
		COMPILE_SCRIPT,
		COMPILE_RUNNABLE,
	};  // type of chunk currently being compiled

	enum ChunkType ct;

	typedef void (Compiler::* ParseFunction) (bool CanAssign);
	
	typedef struct {
		ParseFunction prefix;
		ParseFunction infix;
		Precedence precedence;
	} ParseRule;

	ParseRule RuleTable[NumTokenTypes];
	

	RunnableValue* CurrentBody;
	Chunk *CurrentChunk();

	void error(int e, std::string msg, Token where);
	void ErrorAtPrevious(int e, std::string msg);
	void ErrorAtCurrent(int e, std::string msg);

	void synchronize();
	int CountLines();

	// parsing
	void literal(bool CanAssign);
	void variable(bool CanAssign);
	void call(bool CanAssign);
	void unary(bool CanAssign);
	void binary(bool CanAssign);
	void grouping(bool CanAssign);
	void expression(bool CanAssign);
	void statement(bool CanAssign);
	void declaration(bool CanAssign);

	void VarDeclaration();
	void RunnableDeclaration();

	void ExpressionStatement();
	
	uint8_t block();		// returns code matching type of closing token

	void IfStatement();
	void WhileStatement();
	void RepeatStatement();

	uint8_t ArgumentList();
	std::vector<std::string> ParameterList();

	ParseRule& GetRule(TokenType);
	void ParsePrecedence(Precedence);

	Token advance();
	bool match(TokenType type);
	void consume(TokenType type, std::string ErrorMsg);
	Token Current();
	Token peek(int distance);

	// bytecode
	void EmitByte(uint8_t byte);
	void EmitBytes(uint8_t byte1, uint8_t byte2);
	void EmitReturn();

	short EmitJump(Opcode JumpInstruction);
	void PatchJump(short JumpIndex);
	void PatchLoop(short LoopStart);


	uint8_t SafeAddConstant(Token Constant);
	uint8_t SafeAddConstant(Value *v);  // for objects that have to be defined as values before insertion

	uint8_t AddLocal(Token identifier);
	short ResolveLocal(Token identifier);
};

