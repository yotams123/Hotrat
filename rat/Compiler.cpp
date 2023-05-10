#include "Compiler.h"

Compiler::Compiler(std::vector<Token>& tokens) {
	this->tokens = tokens;
	CurrentToken = tokens.begin();

	for (size_t i = 0; i < NUM_TOKENTYPES; i++) {
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

	RuleTable[LEFT_PAREN] = { &Compiler::grouping, nullptr, PREC_NONE };

	RuleTable[TOKEN_EOF] = { nullptr, nullptr, PREC_END };

	CurrentChunk = new Chunk;
}

Compiler::~Compiler() {
	delete CurrentChunk;
}

Chunk* Compiler::Compile() {
	while (CurrentToken->GetType() != TOKEN_EOF) {
		ParsePrecedence(PREC_NONE);
	}
	return CurrentChunk;
}

void Compiler::literal() {
	TokenType type = CurrentToken->GetType();

	switch (type)
	{
	case NONE:
		EmitByte(OP_NONE);
		break;
	case STRING_LITERAL:
		break;
	case INT_LITERAL:
		EmitBytes(OP_CONSTANT, CurrentChunk->AddConstant(*CurrentToken));
		break;
	case FLOAT_LITERAL:
		break;
	case TRUE:
		EmitByte(OP_TRUE);
		break;
	case FALSE:
		EmitByte(OP_FALSE);
		break;
	default:
		break;
	}

	advance();
}

void Compiler::unary() {
	Token op = advance();

	ParsePrecedence(PREC_UNARY);
	
	switch (op.GetType()) {
	case MINUS:
		EmitByte(OP_NEGATE);
		break;
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
	case PLUS:
		EmitByte(OP_ADD);
		break;
	case MINUS:
		EmitByte(OP_SUB);
		break;
	
	case STAR:
		EmitByte(OP_MULTIPLY);
		break;
	case SLASH:
		EmitByte(OP_DIVIDE);
		break;

	case SHIFT_LEFT:
		EmitByte(OP_SHIFT_LEFT);
		break;
		
	case SHIFT_RIGHT:
		EmitByte(OP_SHIFT_RIGHT);
		break;

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
		std::cerr << "Expected expression"; // TODO
		return;
	}
	(this->*PrefixRule)();

	while (precedence <= GetRule(CurrentToken->GetType()).precedence) {
		rule = GetRule(CurrentToken->GetType());
		ParseFunction InfixRule = rule.infix;
		if (InfixRule == nullptr) {
			std::cerr << "Expected expression"; // TODO
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
	return (CurrentToken++)->GetType() == type;
}

void Compiler::consume(TokenType type, std::string ErrorMsg) {
	if (match(type)) return;

	std::cerr << ErrorMsg; // TODO
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

Chunk::Chunk() {
	code = std::vector<uint8_t>();
	ip = code.begin();

	constants = std::vector<int>();
	enclosing = nullptr;
}

uint8_t Chunk::AddConstant(Token constant) {
	if (constants.size() >= 256) return 0; // TODO throw error

	int value = stoi(constant.GetLexeme());
	constants.push_back(value);

	return constants.size() - 1; // index of constant
}

void Chunk::Append(uint8_t byte) {
	code.push_back(byte);
}

void Chunk::Append(uint8_t byte1, uint8_t byte2) {
	code.push_back(byte1);
	code.push_back(byte2);
}

std::vector<uint8_t>& Chunk::GetCode() {
	return code;
}