#include "Compiler.h"


// TODO protect against invalid assigning in expressions, e.g "a + b = 9"
// TODO functions

Compiler::Compiler(std::vector<Token>& tokens) {
	this->tokens = tokens;
	CurrentTokenOffset = 0;
	
	HadError = false;

	for (size_t i = 0; i < NumTokenTypes; i++) {
		RuleTable[i] = { nullptr, nullptr, PREC_NONE };
	}

	// add actual values to table
	RuleTable[NUM_LITERAL] =	{ &Compiler::literal, nullptr, PREC_LITERAL };
	RuleTable[STRING_LITERAL] = { &Compiler::literal, nullptr, PREC_LITERAL };
	
	RuleTable[TRUE] =	{ &Compiler::literal, nullptr, PREC_LITERAL };
	RuleTable[FALSE] =	{ &Compiler::literal, nullptr, PREC_LITERAL };
	RuleTable[NONE] =	{ &Compiler::literal, nullptr, PREC_LITERAL };

	RuleTable[AND] = { nullptr, &Compiler::binary, PREC_AND };
	RuleTable[OR] = { nullptr, &Compiler::binary, PREC_OR };

	RuleTable[PLUS] =	{ nullptr,			&Compiler::binary, PREC_TERM };
	RuleTable[MINUS] =	{ &Compiler::unary, &Compiler::binary, PREC_TERM };

	RuleTable[STAR] =	{ nullptr, &Compiler::binary, PREC_FACTOR };
	RuleTable[SLASH] =	{ nullptr, &Compiler::binary, PREC_FACTOR };

	RuleTable[SHIFT_LEFT] =		{ nullptr, &Compiler::binary, PREC_BIT };
	RuleTable[SHIFT_RIGHT] =	{ nullptr, &Compiler::binary, PREC_BIT };

	RuleTable[BIT_AND] =	{ nullptr,			&Compiler::binary,	PREC_BIT };
	RuleTable[BIT_OR] =		{ nullptr,			&Compiler::binary,	PREC_BIT };
	RuleTable[BIT_XOR] =	{ nullptr,			&Compiler::binary,	PREC_BIT };
	RuleTable[BIT_NOT] =	{ &Compiler::unary, nullptr,			PREC_BIT };

	RuleTable[DOUBLE_EQUALS] =	{ nullptr, &Compiler::binary, PREC_COMPARE };
	RuleTable[BANG_EQUALS] =	{ nullptr, &Compiler::binary, PREC_COMPARE };
	RuleTable[GREATER_EQUAL] =	{ nullptr, &Compiler::binary, PREC_COMPARE };
	RuleTable[LESS_EQUAL] =		{ nullptr, &Compiler::binary, PREC_COMPARE };
	RuleTable[GREATER] =		{ nullptr, &Compiler::binary, PREC_COMPARE };
	RuleTable[LESS] =			{ nullptr, &Compiler::binary, PREC_COMPARE };

	RuleTable[LEFT_PAREN] = { &Compiler::grouping, nullptr, PREC_NONE };

	RuleTable[BANG] = { &Compiler::unary, nullptr, PREC_UNARY };

	RuleTable[TOKEN_EOF] =		{ nullptr, nullptr, PREC_END };
	RuleTable[TOKEN_NEWLINE] =	{ nullptr, nullptr, PREC_END };

	RuleTable[RAT] = { &Compiler::declaration, nullptr, PREC_NONE };

	RuleTable[IDENTIFIER] = { &Compiler::variable, nullptr, PREC_LITERAL };

	RuleTable[IF] =			{ &Compiler::declaration, nullptr, PREC_ASSIGN };
	RuleTable[WHILE] =		{ &Compiler::declaration, nullptr, PREC_ASSIGN };
	RuleTable[REPEAT] =		{ &Compiler::declaration, nullptr, PREC_ASSIGN };

	CurrentChunk = new Chunk();
}

Compiler::~Compiler() {
	CurrentChunk->ClearConstants();
	delete CurrentChunk;
}

Chunk* Compiler::Compile() {
	while (!match(TOKEN_EOF)) {
		while (match(TOKEN_NEWLINE)) literal(); // to emit the newline byte
		if (match(TOKEN_EOF)) break; // in case script ends with newline
		try {
			declaration();
		}
		catch (int e) {
			if (e >= 100 && e <= 199) {
				synchronize();
			}
		}
	}
	if (HadError) {
		this->CurrentChunk->ClearConstants();
		return nullptr;
	}
	return CurrentChunk;
}


void Compiler::ErrorAtPrevious(int e, std::string msg) {
	error(e, msg, peek(-1));
}

void Compiler::ErrorAtCurrent(int e, std::string msg) {
	error(e, msg, Current());
}

void Compiler::error(int e, std::string msg, Token where) {
	int line = CountLines();
	std::string lexeme =  "'" + where.GetLexeme() + "'";
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
		if (tokens[i].GetType() == TOKEN_NEWLINE) {
			lines++;
		}
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
			uint8_t index = SafeAddConstant(Current());

			EmitBytes(OP_CONSTANT, index);
			break;
		}

		case TRUE:				EmitByte(OP_TRUE);		break;
		case FALSE:				EmitByte(OP_FALSE);		break;
		case NONE:				EmitByte(OP_NONE);		break;

		case TOKEN_NEWLINE:		EmitByte(OP_NEWLINE);	break;
		default:	break;
	}

	advance();
}


uint8_t Compiler::block() {
	while (true) {
		while (match(TOKEN_NEWLINE)) literal();
		Token tok = Current();
		switch (tok.GetType())
		{
			case ENDIF:
			case ELSE:
				return BREAK_IF;

			case ENDWHILE:
				return BREAK_WHILE;

			case ENDFOR:
				return BREAK_FOR;

			case ENDRUNNABLE:
				return BREAK_RUNNABLE;

			case ENDREPEAT:
				return BREAK_REPEAT;

			case TOKEN_EOF:
				return UNCLOSED_BLOCK; break;

			default:
				declaration();
		}
	}
}

void Compiler::variable() {
	Token Identifier = advance();
	uint8_t index = SafeAddConstant(Identifier);

	if (match(EQUALS)) {
		advance();
		expression();
		EmitBytes(OP_SET_GLOBAL, index);

	} else if (match(PLUS_PLUS)) {
		EmitBytes(OP_INC, index);
		advance();
	
	} else if (match(MINUS_MINUS)) {
		EmitBytes(OP_DEC, index);
		advance();

	}
	else if (match(PLUS_ASSIGN)) {
		advance();
		expression();
		EmitBytes(OP_ADD_ASSIGN, index);

	}
	else if (match(MINUS_ASSIGN)) {
		advance();
		expression();
		EmitBytes(OP_SUB_ASSIGN, index);

	}
	else if (match(STAR_ASSIGN)) {
		advance();
		expression();
		EmitBytes(OP_MULTIPLY_ASSIGN, index);

	} else if (match(SLASH_ASSIGN)) {
		advance();
		expression();
		EmitBytes(OP_DIVIDE_ASSIGN, index);

	} else if (match(BIT_AND_ASSIGN)) {
		advance();
		expression();
		EmitBytes(OP_BIT_AND_ASSIGN, index);

	} else if (match(BIT_OR_ASSIGN)) {
		advance();
		expression();
		EmitBytes(OP_BIT_OR_ASSIGN, index);

	} else if (match(BIT_XOR_ASSIGN)) {
		advance();
		expression();
		EmitBytes(OP_BIT_XOR_ASSIGN, index);

	} else if (match(SHIFT_LEFT_ASSIGN)) {
		advance();
		expression();
		EmitBytes(OP_SHIFTL_ASSIGN, index);

	}
	else if (match(SHIFT_RIGHT_ASSIGN)) {
		advance();
		expression();
		EmitBytes(OP_SHIFTR_ASSIGN, index);

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

	short ConditionJump = -1;
	if (op.GetType() == AND) {
		ConditionJump = EmitJump(OP_JUMP_IF_FALSE); // no need to check second condition
	}
	else if (op.GetType() == OR) {
		ConditionJump = EmitJump(OP_JUMP_IF_TRUE);  // no need to check second condition
	}

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

		case AND:
		case OR: {
			PatchJump(ConditionJump);
		}

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
	Token kw = Current();
	switch (kw.GetType()) {
		case RAT: {
			advance();
			VarDeclaration();
			break;
		}

		default: statement();
	}
}

void Compiler::statement() {
	Token kw = Current();

	switch (kw.GetType()) {
		case IF: {
			advance();
			IfStatement();
			break;
		}

		case WHILE: {
			advance();
			WhileStatement();
			break;
		}

		case REPEAT: {
			advance();
			RepeatStatement();
			break;
		}

		default:	ExpressionStatement(); break;
	}
}


void Compiler::ExpressionStatement() {
	expression();
	if (!match(TOKEN_EOF)) {
		consume(TOKEN_NEWLINE, "expected '\\n' after expression");
	}
	EmitByte(OP_POP);
}

void Compiler::VarDeclaration() {
	Token identifier = advance();
	if (identifier.GetType() != IDENTIFIER) {
		ErrorAtPrevious(UNEXPECTED_TOKEN, "Expected identifier after 'rat' keyword");
	}

	uint8_t IdIndex = SafeAddConstant(identifier);
	if (match(EQUALS)) {
		advance();
		expression();
	}
	else {
		EmitByte(OP_NONE);
	}
	EmitBytes(OP_DEFINE_GLOBAL, IdIndex);
}


void Compiler::IfStatement() {
	expression();	// the condition for the block
	consume(COLON, "Expected ':' after expression");
	short SkipIf = EmitJump(OP_JUMP_IF_FALSE);
	short SkipElse = 0;

	uint8_t BlockCode = block();
	switch (BlockCode)
	{
		case BREAK_IF:	break;

		case UNCLOSED_BLOCK:	ErrorAtCurrent(UNCLOSED_BLOCK, "expected 'endif'");
		default:	ErrorAtCurrent(UNEXPECTED_TOKEN, "expected 'endif'");
	}

	if (match(ELSE)) {
		advance();
		consume(COLON, "Expected ':' after else");

		SkipElse = EmitJump(OP_JUMP);
		advance();

		PatchJump(SkipIf);
		EmitByte(OP_POP);  // pop the condition off the stack

		BlockCode = block();
		switch (BlockCode)
		{
			case BREAK_IF:	break;

			case UNCLOSED_BLOCK:	ErrorAtCurrent(UNCLOSED_BLOCK, "expected 'endif'");

			default:	ErrorAtCurrent(UNEXPECTED_TOKEN, "expected 'endif'");
		}

		if (match(ELSE)) ErrorAtCurrent(UNEXPECTED_TOKEN, "Can't have more than one 'else' block");
	}

	advance();
	SkipElse == 0 ? PatchJump(SkipIf) : PatchJump(SkipElse); // end of if block
	EmitByte(OP_POP); // pop the condition off the stack
	return;
}


void Compiler::WhileStatement() {
	short Loopstart = this->CurrentChunk->GetSize() - 1;
	expression();  // loop condition
	consume(COLON, "expected ':' after expression");

	short BreakLoop = EmitJump(OP_JUMP_IF_FALSE);

	uint8_t BlockCode = block();
	switch (BlockCode) {
	case BREAK_WHILE:	advance(); break;

		case UNCLOSED_BLOCK:	ErrorAtCurrent(UNCLOSED_BLOCK, "expected 'endwhile'");

		default:	ErrorAtCurrent(UNEXPECTED_TOKEN, "expected 'endwhile'");
	}
	
	PatchLoop(Loopstart);
	PatchJump(BreakLoop);
}


void Compiler::RepeatStatement() {
	expression();
	consume(COLON, "Expected ':' after expression");

	EmitByte(OP_REPEAT);

	short Loopstart = this->CurrentChunk->GetSize() - 1;

	uint8_t BlockCode = block();
	switch (BlockCode) {
		case BREAK_REPEAT:	advance(); break;

		case UNCLOSED_BLOCK:	ErrorAtCurrent(UNCLOSED_BLOCK, "expected 'endrepeat'");

		default:	ErrorAtCurrent(UNEXPECTED_TOKEN, "expected 'endrepeat'");
	}
	
	EmitByte(OP_END_REPEAT); // will decrement loop value, and jump over the loop instruction if done

	PatchLoop(Loopstart); // will jump to start of loop
}


uint8_t Compiler::SafeAddConstant(Token Constant) {
	// adding a constant to the chunk, wrapped in a try-catch block

	uint8_t index;
	try {
		index = CurrentChunk->AddConstant(Constant);
	}
	catch (std::string e) {
		if (e == "Constants overflow")	ErrorAtCurrent(CONSTANTS_OVERFLOW, "Constants overflow - too many constants in a script/runnable");
		if (e == "Float overflow")		ErrorAtCurrent(FLOAT_OVERFLOW, "Value too large - can't be represented as a number value");
	}

	return index;
}

Compiler::ParseRule& Compiler::GetRule(TokenType type) {
	return RuleTable[type];
}

void Compiler::ParsePrecedence(Precedence precedence) {
	ParseRule rule = GetRule(Current().GetType());
	ParseFunction PrefixRule = rule.prefix;

	if (PrefixRule == nullptr) {
		ErrorAtCurrent(UNEXPECTED_TOKEN, "Expected expression");
		return;
	}
	(this->*PrefixRule)();

	while (precedence <= GetRule(Current().GetType()).precedence) {

		rule = GetRule(Current().GetType());
		ParseFunction InfixRule = rule.infix;
		if (InfixRule == nullptr) {
			ErrorAtCurrent(UNEXPECTED_TOKEN, "Expected expression");
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

	ErrorAtCurrent(UNEXPECTED_TOKEN, ErrorMsg);
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


short Compiler::EmitJump(Opcode JumpInstruction) {
	short index;
	switch (JumpInstruction)
	{
		case OP_JUMP:
		case OP_JUMP_IF_TRUE:
		case OP_JUMP_IF_FALSE:
			EmitByte(JumpInstruction);
			EmitBytes(0, 0);
			index = CurrentChunk->GetSize() - 1;
			return index;

		default:
			ErrorAtCurrent(INTERNAL_ERROR, "");
			break;
	}
}

void Compiler::PatchJump(short JumpIndex) {
	short CurrentIndex = CurrentChunk->GetSize() - 1;
	if (CurrentIndex < JumpIndex) ErrorAtCurrent(INTERNAL_ERROR, "");

	CurrentChunk->PatchJump(JumpIndex, CurrentIndex - JumpIndex);
}

void Compiler::PatchLoop(short LoopStart) {
	EmitByte(OP_LOOP);
	EmitBytes(0, 0);

	short CurrentIndex = CurrentChunk->GetSize() - 1;
	if (CurrentIndex < LoopStart) ErrorAtCurrent(INTERNAL_ERROR, "");

	CurrentChunk->PatchJump(CurrentIndex, CurrentIndex - LoopStart);
}