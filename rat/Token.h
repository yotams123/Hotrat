#pragma once

#include <iostream>
#include <string>
#include <format>
#include <unordered_map>

enum TokenType {
	//data types
	INT_KW, STRING_KW, FLOAT_KW, BOOL, VOID, NONE,

	// braces, parens, brackets	{}, (), []
	LEFT_PAREN, RIGHT_PAREN,
	LEFT_BRACKET, RIGHT_BRACKET,
	LEFT_BRACE, RIGHT_BRACE,

	HASH, // for comments #

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
	STRING_LITERAL, INT_LITERAL, FLOAT_LITERAL,
	TRUE, FALSE,

	// runnables - functions
	RUNNABLE, RETURN, ENDRUNNABLE,

	// rats - classes
	RAT, THIS, ENDRAT,

	// variables
	IDENTIFIER, EQUALS,
	COLD, /* constant */
	BANG, /* ! */

	TOKEN_EOF
};

class Token {
private:
	TokenType type;
	std::string lexeme;
	
public:
	Token(TokenType, std::string);
	std::string ToString();
	TokenType GetType();
};
