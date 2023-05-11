#include "Compiler.h"

Compiler::Compiler(std::vector<Token>& tokens) {
	this->tokens = tokens;
	CurrentToken = tokens.begin();
	
	HadError = false;

	for (size_t i = 0; i < NumTokenTypes; i++) {
		RuleTable[i] = { nullptr, nullptr, PREC_NONE };
	}

	// add actual values to table
	RuleTable[INT_LITERAL] = { &Compiler::literal, nullptr, PREC_LITERAL };
	RuleTable[FLOAT_LITERAL] = { &Compiler::literal, nullptr, PREC_LITERAL };

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


	RuleTable[LEFT_PAREN] = { &Compiler::grouping, nullptr, PREC_NONE };

	RuleTable[TOKEN_EOF] = { nullptr, nullptr, PREC_END };
	RuleTable[TOKEN_NEWLINE] = { nullptr, nullptr, PREC_END };

	CurrentChunk = new Chunk();
}

Compiler::~Compiler() {
	delete CurrentChunk;
}

Chunk* Compiler::Compile() {
	while (!match(TOKEN_EOF)) {
		try {
			ParsePrecedence(PREC_NONE);
		}
		catch (int e) {
			if (e == COMPILE_ERROR) {
				synchronize();
			}
		}
		if (match(TOKEN_NEWLINE)) advance();
	}
	if (HadError) return nullptr;

	CurrentChunk->SyncIP();
	return CurrentChunk;
}


void Compiler::error(std::string msg) {
	int line = CountLines();
	std::cout << "[Compilation error in line " << line << ", at '" << CurrentToken->GetLexeme() 
		<< "' ]: " << msg << "\n";
	HadError = true;
	throw COMPILE_ERROR;
}

void Compiler::synchronize() {
	while (!match(TOKEN_EOF) && !match(TOKEN_NEWLINE)) {
		advance();
	}
	return;
}

int Compiler::CountLines() {
	int lines = 1;
	std::vector<Token>::iterator i = tokens.begin();
	while (i < CurrentToken) {
		if (i->GetType() == TOKEN_NEWLINE) lines++;
		i++;
	}
	return lines;
}

void Compiler::literal() {
	TokenType type = CurrentToken->GetType();

	switch (type)
	{
	case NONE:				EmitByte(OP_NONE);													break;
	case STRING_LITERAL:																		break;
	case INT_LITERAL: {
		uint8_t index = CurrentChunk->AddConstant(*CurrentToken);
		if (index == -1) error("Constants table overflow - too many constants");
		EmitBytes(OP_CONSTANT, index);	break;
	}
	case FLOAT_LITERAL:																			break;
	case TRUE:				EmitByte(OP_TRUE);													break;
	case FALSE:				EmitByte(OP_FALSE);													break;
	default:	break;
	}

	advance();
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

Compiler::ParseRule& Compiler::GetRule(TokenType type) {
	return RuleTable[type];
}

void Compiler::ParsePrecedence(Precedence precedence) {
	ParseRule rule = GetRule(CurrentToken->GetType());
	ParseFunction PrefixRule = rule.prefix;

	if (PrefixRule == nullptr) {
		error("Expected expression");
		return;
	}
	(this->*PrefixRule)();

	while (precedence <= GetRule(CurrentToken->GetType()).precedence) {
		rule = GetRule(CurrentToken->GetType());
		ParseFunction InfixRule = rule.infix;
		if (InfixRule == nullptr) {
			error("Expected expression");
			return;
		}
		(this->*InfixRule)();
	}
	return;
}


Token Compiler::advance() {
	return *(CurrentToken++);
}

bool Compiler::match(TokenType type) {
	return (CurrentToken)->GetType() == type;
}

void Compiler::consume(TokenType type, std::string ErrorMsg) {
	if (match(type)) {
		advance();
		return;
	}

	error(ErrorMsg);
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
