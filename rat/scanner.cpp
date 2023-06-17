#include "scanner.h"

Scanner::Scanner(std::string src) {
	current = 0;
	start = 0;

	this->src = src;
	this->tokens = std::vector<Token>();

	line = 1;
	HadError = false;
}

Scanner::~Scanner(){}

std::vector<Token>& Scanner::ScanTokens() {
	Token token = Token(TOKEN_EOF, ""); // placeholder value
	do { 
		token = ScanToken();
		tokens.push_back(token);
	} while (token.GetType() != TOKEN_EOF);
	
	if (HadError) {
		tokens.clear();
	}
	return tokens;
}

Token Scanner::ScanToken() {
	SkipWhiteSpace();

	start = current; // start of token
	char c = advance();

	switch (c) {
		case '\0':	return Token(TOKEN_EOF, "");

		case 'a': if (CheckWord("nd"))	return Token(AND, "and");	break;
		case 'c': if (CheckWord("old")) return Token(COLD, "cold"); break;

		case 'e': {
			if (MatchString("nd")) {
				if (CheckWord("for"))		return Token(ENDFOR, "endfor");
				if (CheckWord("while"))		return Token(ENDWHILE, "endwhile");
				if (CheckWord("if"))		return Token(ENDIF, "endif");
				if (CheckWord("runnable"))	return Token(ENDRUNNABLE, "endrunnable");
				if (CheckWord("rat"))		return Token(ENDRAT, "endrat");
				if (CheckWord("repeat"))	return Token(ENDREPEAT, "endrepeat");
			}
			else if (CheckWord("lse")) return Token(ELSE, "else");

			break;
		}

		case 'f': {
			if (CheckWord("or"))	return Token(FOR, "for");
			if (CheckWord("alse"))	return Token(FALSE, "false");
			break; 
		}

		case 'i': {
			if (CheckWord("f"))		return Token(IF, "if");
			if (CheckWord("n"))		return Token(IN, "in");
			break; 
		}

		case 'n': if (CheckWord("one"))		return Token(NONE, "none");			break;

		case 'o': if (CheckWord("r"))		return Token(OR, "or");				break;
		case 'r': {
			if (CheckWord("eturn"))	return Token(RETURN, "return");
			if (CheckWord("at")) return Token(RAT, "rat");
			if (CheckWord("unnable")) return Token(RUNNABLE, "runnable");
			if (CheckWord("epeat")) return Token(REPEAT, "repeat");
			
			break;
		}
		
		case 't': { 
			if (CheckWord("his")) return Token(THIS, "this");
			if (CheckWord("rue")) return Token(TRUE, "true");
			break;
		}
		
		case 'w': if (CheckWord("hile"))	return Token(WHILE, "while");	break;
		case 'x': if (CheckWord("or"))		return Token(XOR, "xor");		break;

		case '.':	return Token(DOT, ".");
		case ',':	return Token(COMMA, ",");

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
		case '#': return Comment();
		case ':': return Token(COLON, ":");
	}

	if (isalpha((uint8_t)c) || c == '_') {
		return Identifier();
	}
	else if (isdigit((uint8_t)c)) {
		return Number();
	}
	else {
		std::string msg = "Unidentified character";
		error(msg);
		return Token(TOKEN_ERROR, msg);
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
	}
	return Token(NUM_LITERAL, src.substr(start, current - start));
}

Token Scanner::String() {
	int newline_toks = 0;
	while (peek(0) != '"' && !IsAtEnd()) { 
		if (peek(0) == '\n') {
			newline_toks++;
			line++;
		}
		advance(); 
	}


	if (IsAtEnd()) {
		error("Unclosed string");
		tokens.push_back(Token(TOKEN_EOF, ""));
	}
	advance(); // get rid of closing ""

	Token str = Token(STRING_LITERAL, src.substr(start + 1, current - start - 2));
	if (newline_toks <= 0) {
		return (str);
	}
	else {
		tokens.push_back(str);
		for (int i = 0; i < newline_toks - 1; i++) {
			tokens.push_back(Token(TOKEN_NEWLINE, "\n"));
		}
		return Token(TOKEN_NEWLINE, "\n");
	}
}

Token Scanner::Identifier() {
	while (isalnum((uint8_t)peek(0)) || peek(0) == '_') { advance(); }
	return Token(IDENTIFIER, src.substr(start, current - start));
}

Token Scanner::Comment() {
	while (!(match('\n') || IsAtEnd())) advance();
	if (IsAtEnd()) {
		return Token(TOKEN_EOF, "");
	}
	return Token(HASH, "#"); // compiler will discard
}

void Scanner::SkipWhiteSpace() {
	while (true) {
		start = current;
		switch (peek(0)) {
			case '\n':
				tokens.push_back(Token(TOKEN_NEWLINE, "\n"));
				line++;
			case '\t':
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

char Scanner::peek(size_t distance) {
	return src[current + distance];
}

bool Scanner::MatchString(std::string target) {
	if (src.substr(current, target.length()) == target) {
		for (int i = 0; i < target.length(); i++) advance();
		return true;
	}

	return false;
}

bool Scanner::match(char c) {
	if (peek(0) == c) {
		advance();
		return true;
	}
	return false;
}

bool Scanner::IsAtEnd() {
	return current >= src.length();
}


void Scanner::error(std::string ErrorMsg) {
	std::cerr << "[Error in line " << line << " at '" << peek(-1) << "']: " << ErrorMsg << std::endl;
	HadError = true;
}