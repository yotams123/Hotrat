#include "scanner.h"

Scanner::Scanner(std::string src) {
	current = 0;
	start = 0;
	line = 1;

	this->src = src;
	this->tokens = std::vector<Token>();
}

std::vector<Token> Scanner::ScanTokens() {
	Token token = Token(TOKEN_EOF, "");
	do { 
		token = ScanToken();
		tokens.push_back(token);
	} while (token.GetType() != TOKEN_EOF);

	return tokens;
}

Token Scanner::ScanToken() {
	SkipWhiteSpace();

	start = current; // start of token
	char c = advance();

	switch (c) {
		case '\0':	return Token(TOKEN_EOF, "");
		case 'a': if (CheckWord("nd"))	return Token(AND, "and");	break;
		case 'b': if (CheckWord("ool")) return Token(BOOL, "bool"); break;
		case 'c': if (CheckWord("old")) return Token(COLD, "cold"); break;

		case 'e': {
			if (CheckWord("nd")) {
				if (CheckWord("for"))		return Token(ENDFOR, "endfor");
				if (CheckWord("while"))		return Token(ENDWHILE, "endwhile");
				if (CheckWord("if"))		return Token(ENDIF, "endif");
				if (CheckWord("runnable"))	return Token(ENDRUNNABLE, "endrunnable");
				if (CheckWord("rat"))		return Token(ENDRAT, "endrat");
			}
			else if (CheckWord("lse")) return Token(ELSE, "else");

			break;
		}

		case 'f': {
			if (CheckWord("or"))	return Token(FOR, "for");
			if (CheckWord("alse"))	return Token(FALSE, "false");
			if (CheckWord("loat"))	return Token(FLOAT_KW, "float");
			break; 
		}

		case 'i': {
			if (CheckWord("f"))		return Token(IF, "if");
			if (CheckWord("n"))		return Token(IN, "in");
			if (CheckWord("nt"))	return Token(INT_KW, "int");
			break; 
		}

		case 'n': if (CheckWord("one"))		return Token(NONE, "none");			break;
		case 'o': if (CheckWord("r"))		return Token(OR, "or");				break;
		case 'r': if (CheckWord("eturn"))	return Token(RETURN, "return");		break;
		case 'R': if (CheckWord("unnable")) return Token(RUNNABLE, "Runnable"); break;
		case 's': if (CheckWord("tring"))	return Token(STRING_KW, "string");	break;
		
		case 't': { 
			if (CheckWord("his")) return Token(THIS, "this");
			if (CheckWord("rue")) return Token(TRUE, "true");
			break;
		}
		
		case 'v': if (CheckWord("oid"))		return Token(VOID, "void");		break;
		case 'w': if (CheckWord("hile"))	return Token(WHILE, "while");	break;
		case 'x': if (CheckWord("or"))		return Token(XOR, "xor");		break;

		case '{': return Token(LEFT_BRACE, "{");		break;
		case '}': return Token(RIGHT_BRACE, "}");		break;
		case '[': return Token(LEFT_BRACKET, "[");		break;
		case ']': return Token(RIGHT_BRACKET, "]");		break;
		case '(': return Token(LEFT_PAREN, "(");		break;
		case ')': return Token(RIGHT_PAREN, ")");		break;

		case '&': {
			if (match('=')) {
				return Token(BIT_AND_ASSIGN, "&=");
			}
			else return Token(BIT_AND, "&");
		}

		case '|': {
			if (match('=')) {
				return Token(BIT_OR_ASSIGN, "|=");
			}
			else return Token(BIT_OR, "|");
		}

		case '^': {
			if (match('=')) {
				return Token(BIT_XOR_ASSIGN, "^=");
			}
			else return Token(BIT_XOR, "^");
		}

		case '~': return Token(BIT_NOT, "~");

		case '!': {
			if (match('=')) {
				return Token(BANG_EQUALS, "!=");
			}
			else return Token(BANG, "!");
		}

		case '=': {
			if (match('=')) {
				return Token(DOUBLE_EQUALS, "==");
			}
			else return Token(EQUALS, "=");
		}

		case '<': {
			if (match('<')) {
				if (match('=')) {
					return Token(SHIFT_LEFT_ASSIGN, "<<=");
				}
				return Token(SHIFT_LEFT, "<<");
			}
			if (match('=')){
				return Token(LESS_EQUAL, "<=");
			} 
			return Token(LESS, "<");
		}

		case '>': {
			if (match('>')) {
				if (match('=')) {
					return Token(SHIFT_RIGHT_ASSIGN, ">>=");
				}
				return Token(SHIFT_RIGHT, ">>");
			}
			if (match('=')) {
				return Token(GREATER_EQUAL, ">=");
			}
			return Token(GREATER, ">");
		}

		case '+': {
			if (match('=')) {
				return Token(PLUS_ASSIGN, "+=");
			}
			if (match('+')) {
				return Token(PLUS_PLUS, "++");
			}
			return Token(PLUS, "+");
		}

		case '-': {
			if (match('=')) {
				return Token(MINUS_ASSIGN, "-=");
			}
			if (match('-')) {
				return Token(MINUS_MINUS, "--");
			}
			return Token(MINUS, "-");
		}

		case '*': {
			if (match('=')) {
				return Token(STAR_ASSIGN, "*=");
			}
			else return Token(STAR, "*");
		}

		case '/': {
			if (match('=')) {
				return Token(SLASH_ASSIGN, "/=");
			}
			else return Token(SLASH, "/");
		}

		case '"': return String();
	}

	if (isalpha(c) || c == '_') {
		return Identifier();
	}
	else if (isdigit(c)) {
		return Number();
	}
	else {
		error("Unidentified character");
	}

}

bool Scanner::CheckWord(std::string rest) {
	for (int i = 0; i < rest.length(); i++) {
		if (peek(i) != rest[i]) return false;
	}
	char c = peek(rest.length());
	if (isalnum(c) || c == '_') return false; // still keyword

	for (int i = 0; i < rest.length(); i++) advance();
	return true;
}

Token Scanner::Number() {
	while (isdigit(peek(0))) { advance(); }
	
	if (peek(0) == '.') {
		advance();
		while (isdigit(peek(0))) { advance(); }
		return Token(FLOAT_LITERAL, src.substr(start, current - start));
	}
	return Token(INT_LITERAL, src.substr(start, current - start));
}

Token Scanner::String() {
	while (peek(0) != '"') { advance(); }
	return Token(STRING_LITERAL, src.substr(start, current - start));
}

Token Scanner::Identifier() {
	while (isalnum(peek(0)) || peek(0) == '_') { advance(); }
	return Token(IDENTIFIER, src.substr(start, current - start));
}

void Scanner::SkipWhiteSpace() {
	while (true) {
		start = current;
		switch (peek(0)) {
		case '\t':
		case '\n':
		case ' ':
			advance();
			break;
		default:
			return;
		}
	}
}

char Scanner::advance() {
	return src[current++];
}

char Scanner::peek(int level) {
	return src[current + level];
}

bool Scanner::match(char c) {
	if (peek(0) == c) {
		advance();
		return true;
	}
	return false;
}

void Scanner::error(std::string ErrorMsg) {
	int line = CountLines();
	std::cerr << "[Error in line " << line << "]" << ErrorMsg << std::endl;
}

int Scanner::CountLines() {
	int lines = 0;
	int runner = 0;

	while (runner < current) {
		if (src[runner] == '\n') lines++;
		runner++;
	}
	return lines;
}