#include "Token.h"

Token::Token(TokenType type, std::string lexeme) {
	this->type = type;
	this->lexeme = lexeme;
}

std::string Token::ToString() {
	switch (this->type)
	{
		case STRING_LITERAL:
			return " { String literal,\tvalue = \"" + lexeme + "\"}";
		case NUM_LITERAL:
			return " { Number literal, \tvalue = " + lexeme + " }";
		case TOKEN_EOF:
			return " { EOF token}";
		case TOKEN_ERROR:
			return " { Error token, \tmsg = \"" + lexeme + "\" }";
		case IDENTIFIER:
			return " { Identifier,\tvalue = " + lexeme + " }";
		default:
			return " { keyword or operation, \tvalue = " + lexeme + " }";
	}
}

TokenType Token::GetType() { return type; }
std::string& Token::GetLexeme() { return lexeme; }