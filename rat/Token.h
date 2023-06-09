#pragma once

#include <iostream>
#include <string>
#include <format>
#include <unordered_map>

enum TokenType {
	//data types
	NONE,

	// braces, parens, brackets	{}, (), []
	LEFT_PAREN, RIGHT_PAREN,
	LEFT_BRACKET, RIGHT_BRACKET,
	LEFT_BRACE, RIGHT_BRACE,

	COLON,

	// loops and conditions
	IF, ELSE, WHILE, FOR, IN, REPEAT,
	ENDIF, ENDWHILE, ENDFOR, ENDREPEAT,

	// logical operators
	AND,
	OR,
	XOR,

	DOUBLE_EQUALS, BANG_EQUALS,
	GREATER, GREATER_EQUAL,
	LESS, LESS_EQUAL,

	// mathmatical operators
	PLUS, MINUS,
	PLUS_PLUS, MINUS_MINUS,
	STAR, SLASH,
	SHIFT_LEFT, SHIFT_RIGHT,

	PLUS_ASSIGN, MINUS_ASSIGN,				// +=, -=
	STAR_ASSIGN, SLASH_ASSIGN,				// *=, /=
	SHIFT_LEFT_ASSIGN, SHIFT_RIGHT_ASSIGN,	// <<=, >>=

	// bitwise operators
	BIT_AND, BIT_OR,
	BIT_XOR, BIT_NOT,

	BIT_AND_ASSIGN, BIT_OR_ASSIGN,	// &=, |=
	BIT_XOR_ASSIGN,					// ^= 

	// literals
	STRING_LITERAL, NUM_LITERAL,
	TRUE, FALSE,

	// runnables - functions
	RUNNABLE, RETURN, ENDRUNNABLE,

	// rats - classes
	RAT, THIS, ENDRAT,

	// variables
	IDENTIFIER, EQUALS,
	COLD, /* constant */
	BANG, /* ! */

	COMMA, DOT,

	TOKEN_ERROR, TOKEN_NEWLINE, TOKEN_EOF,
};

const int NumTokenTypes = TOKEN_EOF + 1;

class Token {
private:
	TokenType type;
	std::string lexeme;
	
public:
	Token(TokenType, std::string);
	std::string ToString();

	TokenType GetType();
	std::string& GetLexeme();
};
