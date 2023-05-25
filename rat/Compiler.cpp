#include "Compiler.h"

Compiler::Compiler(std::vector<Token>& tokens) {
	this->tokens = tokens;
	CurrentTokenOffset = 0;
	
	HadError = false;

	for (size_t i = 0; i < NumTokenTypes; i++) {
		RuleTable[i] = { nullptr, nullptr, PREC_NONE };
	}

	// add actual values to table
	RuleTable[NUM_LITERAL] = { &Compiler::literal, nullptr, PREC_LITERAL };
	RuleTable[STRING_LITERAL] = { &Compiler::literal, nullptr, PREC_LITERAL };
	
	RuleTable[TRUE] = { &Compiler::literal, nullptr, PREC_LITERAL };
	RuleTable[FALSE] = { &Compiler::literal, nullptr, PREC_LITERAL };

	RuleTable[PLUS] = { nullptr, &Compiler::binary, PREC_TERM };
	RuleTable[MINUS] = { &Compiler::unary, &Compiler::binary, PREC_TERM };

	RuleTable[STAR] = { nullptr, &Compiler::binary, PREC_FACTOR };
	RuleTable[SLASH] = { nullptr, &Compiler::binary, PREC_FACTOR };

	RuleTable[SHIFT_LEFT] = { nullptr, &Compiler::binary, PREC_BIT };
	RuleTable[SHIFT_RIGHT] = { nullptr, &Compiler::binary, PREC_BIT };

	RuleTable[BIT_AND] = { nullptr, &Compiler::binary, PREC_BIT };
	RuleTable[BIT_OR] = { nullptr, &Compiler::binary, PREC_BIT };
	RuleTable[BIT_XOR] = { nullptr, &Compiler::binary, PREC_BIT };
	RuleTable[BIT_NOT] = { &Compiler::unary, nullptr, PREC_BIT };

	RuleTable[DOUBLE_EQUALS] = { nullptr, &Compiler::binary, PREC_COMPARE };
	RuleTable[BANG_EQUALS] = { nullptr, &Compiler::binary, PREC_COMPARE };
	RuleTable[GREATER_EQUAL] = { nullptr, &Compiler::binary, PREC_COMPARE };
	RuleTable[LESS_EQUAL] = { nullptr, &Compiler::binary, PREC_COMPARE };
	RuleTable[GREATER] = { nullptr, &Compiler::binary, PREC_COMPARE };
	RuleTable[LESS] = { nullptr, &Compiler::binary, PREC_COMPARE };

	RuleTable[LEFT_PAREN] = { &Compiler::grouping, nullptr, PREC_NONE };

	RuleTable[BANG] = { &Compiler::unary, nullptr, PREC_UNARY };

	RuleTable[TOKEN_EOF] = { nullptr, nullptr, PREC_END };
	RuleTable[TOKEN_NEWLINE] = { nullptr, nullptr, PREC_END };

	RuleTable[RAT] = { &Compiler::declaration, nullptr, PREC_NONE };

	RuleTable[IDENTIFIER] = { &Compiler::variable, nullptr, PREC_LITERAL };
	CurrentChunk = new Chunk();
}

Compiler::~Compiler() {
	delete CurrentChunk;
}

Chunk* Compiler::Compile() {
	while (!match(TOKEN_EOF)) {
		try {
			expression();
		}
		catch (int e) {
			if (e >= 100 && e <= 199) {
				synchronize();
			}
		}
		while (match(TOKEN_NEWLINE)) literal(); // to emit the newline byte
	}
	if (HadError) return nullptr;

	CurrentChunk->SyncIP();
	return CurrentChunk;
}


void Compiler::error(int e, std::string msg) {
	int line = CountLines();
	std::string lexeme =  "'" + Current().GetLexeme() + "'";
	if (lexeme == "'\n'") lexeme = "end";

	std::cout << "[Compilation error in line " << line << ", at " << lexeme	<< " ]: " << msg << "\n";
	HadError = true;
	throw e;
}

void Compiler::synchronize() {
	while (!match(TOKEN_EOF) && !match(TOKEN_NEWLINE)) {
		advance();
	}
	return;
}

int Compiler::CountLines() {
	int lines = 1;
	int i = 0;
	while (i != CurrentTokenOffset) {
		if (tokens[i].GetType() == TOKEN_NEWLINE) lines++;
		i++;
	}
	return lines;
}

void Compiler::literal() {
	TokenType type = Current().GetType();

	switch (type)
	{
		case STRING_LITERAL:
		case NUM_LITERAL: {
			uint8_t index = CurrentChunk->AddConstant(Current());
			if (index == -1) error(CONSTANTS_OVERFLOW, "Constants table overflow - too many constants");
			EmitBytes(OP_CONSTANT, index);
			break;
		}

		case TRUE:				EmitByte(OP_TRUE);		break;
		case FALSE:				EmitByte(OP_FALSE);		break;

		case TOKEN_NEWLINE:		EmitByte(OP_NEWLINE);	break;
		default:	break;
	}

	advance();
}


void Compiler::variable() {
	Token Identifier = advance();
	uint8_t index = CurrentChunk->AddConstant(Identifier);
	
	if (match(EQUALS)) {
		advance();
		expression();
		EmitBytes(OP_SET_GLOBAL, index);
	}
	else {
		EmitBytes(OP_GET_GLOBAL, index);
	}
}

void Compiler::unary() {
	Token op = advance();

	ParsePrecedence(PREC_UNARY);
	
	switch (op.GetType()) {
	case MINUS:	EmitByte(OP_NEGATE); break;
	case BANG:  EmitByte(OP_NOT);	 break;

	default: break;
	}

}

void Compiler::binary() {
	Token op = advance();

	ParseRule rule = GetRule(op.GetType());
	Precedence ToParse = (Precedence)(rule.precedence + 1);

	ParsePrecedence(ToParse);
	switch (op.GetType())
	{
		case PLUS:			EmitByte(OP_ADD);			break;
		case MINUS:			EmitByte(OP_SUB);			break;
		case STAR:			EmitByte(OP_MULTIPLY);		break;
		case SLASH:			EmitByte(OP_DIVIDE);		break;

		case SHIFT_LEFT:	EmitByte(OP_SHIFT_LEFT);	break;	
		case SHIFT_RIGHT:	EmitByte(OP_SHIFT_RIGHT);	break;

		case BIT_AND:		EmitByte(OP_BIT_AND);		break;
		case BIT_OR:		EmitByte(OP_BIT_OR);		break;
		case BIT_XOR:		EmitByte(OP_BIT_XOR);		break;

		case DOUBLE_EQUALS:	EmitByte(OP_EQUALS);			break;
		case BANG_EQUALS:	EmitBytes(OP_EQUALS, OP_NOT);	break;
		case GREATER:		EmitByte(OP_GREATER);			break;
		case GREATER_EQUAL:	EmitBytes(OP_LESS, OP_NOT);		break;
		case LESS:			EmitByte(OP_LESS);				break;
		case LESS_EQUAL:	EmitBytes(OP_GREATER, OP_NOT);	break;

		default:
			break;
	}
}

void Compiler::grouping() {
	advance(); // consume left parenthese
	expression();
	consume(RIGHT_PAREN, (std::string)"Expected ')' after expression");
}

void Compiler::expression(){
	ParsePrecedence(PREC_ASSIGN);
}

void Compiler::declaration() {
	Token kw = advance();
	switch (kw.GetType()) {
		case RAT: {
			VarDeclaration();
			break;
		}
	}
}

void Compiler::VarDeclaration() {
	Token identifier = advance();
	uint8_t IdIndex = CurrentChunk->AddConstant(identifier);
	if (match(EQUALS)) {
		advance();
		expression();
	}
	else {
		EmitByte(OP_NONE);
	}
	EmitBytes(OP_DEFINE_GLOBAL, IdIndex);
}

Compiler::ParseRule& Compiler::GetRule(TokenType type) {
	return RuleTable[type];
}

void Compiler::ParsePrecedence(Precedence precedence) {
	ParseRule rule = GetRule(Current().GetType());
	ParseFunction PrefixRule = rule.prefix;

	if (PrefixRule == nullptr) {
		error(UNEXPECTED_TOKEN, "Expected expression");
		return;
	}
	(this->*PrefixRule)();

	while (precedence <= GetRule(Current().GetType()).precedence) {

		rule = GetRule(Current().GetType());
		ParseFunction InfixRule = rule.infix;
		if (InfixRule == nullptr) {
			error(UNEXPECTED_TOKEN, "Expected expression");
		}
		(this->*InfixRule)();
	}
	return;
}


Token Compiler::advance() {
	return tokens[CurrentTokenOffset++];
}

Token Compiler::Current() {
	return tokens[CurrentTokenOffset];
}

Token Compiler::peek(int distance) {
	return tokens[CurrentTokenOffset + distance];
}

bool Compiler::match(TokenType type) {
	return Current().GetType() == type;
}

void Compiler::consume(TokenType type, std::string ErrorMsg) {
	if (match(type)) {
		advance();
		return;
	}

	error(UNEXPECTED_TOKEN, ErrorMsg);
}

void Compiler::EmitByte(uint8_t byte) {
	CurrentChunk->Append(byte);
}

void Compiler::EmitBytes(uint8_t byte1, uint8_t byte2) {
	CurrentChunk->Append(byte1, byte2);
}

void Compiler::EmitReturn() {
	CurrentChunk->Append(OP_RETURN);
}
