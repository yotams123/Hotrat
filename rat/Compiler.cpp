#include "Compiler.h"

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

	RuleTable[AND] =	{ nullptr, &Compiler::binary, PREC_AND };
	RuleTable[OR] =		{ nullptr, &Compiler::binary, PREC_OR };

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

	RuleTable[LEFT_PAREN] = { &Compiler::grouping, &Compiler::call, PREC_LITERAL};

	RuleTable[BANG] =		{ &Compiler::unary, nullptr, PREC_UNARY };

	RuleTable[TOKEN_EOF] =		{ nullptr, nullptr, PREC_END };
	RuleTable[TOKEN_NEWLINE] =	{ nullptr, nullptr, PREC_END };

	RuleTable[RAT] =		{ &Compiler::declaration, nullptr, PREC_NONE };

	RuleTable[IDENTIFIER] = { &Compiler::variable, nullptr, PREC_LITERAL };

	RuleTable[IF] =			{ &Compiler::declaration, nullptr, PREC_ASSIGN };
	RuleTable[WHILE] =		{ &Compiler::declaration, nullptr, PREC_ASSIGN };
	RuleTable[REPEAT] =		{ &Compiler::declaration, nullptr, PREC_ASSIGN };

	CurrentBody = new RunnableValue(new Chunk, 0, "script");
}

Compiler::~Compiler() {
}

RunnableValue* Compiler::Compile() {
	while (!match(TOKEN_EOF)) {
		while (match(TOKEN_NEWLINE)) literal(true); // to emit the newline byte
		if (match(TOKEN_EOF)) break; // in case script ends with newline
		try {
			declaration(true);
		}
		catch (int e) {
			if (e >= 100 && e <= 199) {
				synchronize();
			}
		}
	}
	if (HadError) {
		delete this->CurrentBody;
		return nullptr;
	}
	return this->CurrentBody;
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

void Compiler::literal(bool CanAssign) {
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
		while (match(TOKEN_NEWLINE)) literal(true);
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

			case RUNNABLE:	
				ErrorAtCurrent(BLOCKED_RUNNABLE, "Can't define a runnable inside a block");
			default:
				declaration(true);
		}
	}
}

void Compiler::variable(bool CanAssign) {
	Token Identifier = advance();
	uint8_t index;
	enum VarType {global, local};
	VarType CurrentVar = global;

	if (ct == COMPILE_RUNNABLE) {
		short sindex = ResolveLocal(Identifier);

		if (sindex == -1) {
			sindex = SafeAddConstant(Identifier); // saved as short so it can represent negative values
		}
		else  CurrentVar = local;

		index = (uint8_t)sindex;
	}
	else if (ct == COMPILE_SCRIPT) {
		index = SafeAddConstant(Identifier);
	}

	Opcode op;
	if (CanAssign) {
		if (match(EQUALS)) {
			advance();
			expression(true);
			if (CurrentVar == local) op = OP_SET_LOCAL;
			else if (CurrentVar == global) op = OP_SET_GLOBAL;

		}
		else if (match(PLUS_PLUS)) {
			if (CurrentVar == local) op = OP_INC_LOCAL;
			else if (CurrentVar == global) op = OP_INC_GLOBAL;
			advance();

		}
		else if (match(MINUS_MINUS)) {
			if (CurrentVar == local) op = OP_DEC_LOCAL;
			else if (CurrentVar == global) op = OP_DEC_GLOBAL;
			advance();

		}
		else if (match(PLUS_ASSIGN)) {
			advance();
			expression(true);
			if (CurrentVar == local) op = OP_ADD_ASSIGN_LOCAL;
			else if (CurrentVar == global) op = OP_ADD_ASSIGN_GLOBAL;

		}
		else if (match(MINUS_ASSIGN)) {
			advance();
			expression(true);
			if (CurrentVar == local) op = OP_SUB_ASSIGN_LOCAL;
			else if (CurrentVar == global) op = OP_SUB_ASSIGN_GLOBAL;

		}
		else if (match(STAR_ASSIGN)) {
			advance();
			expression(true);

			if (CurrentVar == local) op = OP_MULTIPLY_ASSIGN_LOCAL;
			else if (CurrentVar == global) op = OP_MULTIPLY_ASSIGN_GLOBAL;

		}
		else if (match(SLASH_ASSIGN)) {
			advance();
			expression(true);

			if (CurrentVar == local) op = OP_DIVIDE_ASSIGN_LOCAL;
			else if (CurrentVar == global) op = OP_DIVIDE_ASSIGN_GLOBAL;

		}
		else if (match(BIT_AND_ASSIGN)) {
			advance();
			expression(true);
			if (CurrentVar == local) op = OP_BIT_AND_ASSIGN_LOCAL;
			else if (CurrentVar == global) op = OP_BIT_AND_ASSIGN_GLOBAL;

		}
		else if (match(BIT_OR_ASSIGN)) {
			advance();
			expression(true);
			if (CurrentVar == local) op = OP_BIT_OR_ASSIGN_LOCAL;
			else if (CurrentVar == global) op = OP_BIT_OR_ASSIGN_GLOBAL;

		}
		else if (match(BIT_XOR_ASSIGN)) {
			advance();
			expression(true);

			if (CurrentVar == local) op = OP_BIT_XOR_ASSIGN_LOCAL;
			else if (CurrentVar == global) op = OP_BIT_XOR_ASSIGN_GLOBAL;

		}
		else if (match(SHIFT_LEFT_ASSIGN)) {
			advance();
			expression(true);
			if (CurrentVar == local) op = OP_SHIFTL_ASSIGN_LOCAL;
			else if (CurrentVar == global) op = OP_SHIFTL_ASSIGN_GLOBAL;

		}
		else if (match(SHIFT_RIGHT_ASSIGN)) {
			advance();
			expression(true);
			if (CurrentVar == local) op = OP_SHIFTR_ASSIGN_LOCAL;
			else if (CurrentVar == global) op = OP_SHIFTR_ASSIGN_GLOBAL;

		} else {
			if (CurrentVar == local) op = OP_GET_LOCAL;
			else if (CurrentVar == global) op = OP_GET_GLOBAL;
		}
	}
	else {
		if (CurrentVar == local) op = OP_GET_LOCAL;
		else if (CurrentVar == global) op = OP_GET_GLOBAL;
	}

	EmitBytes(op, index);
	
}

void Compiler::call(bool CanAssign) {
	Token name = peek(-1);

	RunnableValue* r = nullptr;
	if (ct == COMPILE_SCRIPT) {
		short RunnableIndex = CurrentChunk()->FindRunnable(name);
		
		if (RunnableIndex == -1) ErrorAtPrevious(UNDEFINED_RUNNABLE, "Undefined runnable '" + name.GetLexeme() + "'\n");
		
		r = (RunnableValue *)(CurrentChunk()->ReadConstant((uint8_t)RunnableIndex).GetObject());
	}
	else if (ct == COMPILE_RUNNABLE) {
		Chunk* global = this->CurrentBody->GetEnclosing()->GetChunk();

		short RunnableIndex = global->FindRunnable(name);

		if (RunnableIndex == -1) ErrorAtPrevious(UNDEFINED_RUNNABLE, "Undefined runnable '" + name.GetLexeme() + "'\n");

		r = r = (RunnableValue*)(global->ReadConstant((uint8_t)RunnableIndex).GetObject());
	}
	if (r == nullptr) error(INTERNAL_ERROR, "", name);

	advance();	// advance over opening parenthesis

	uint8_t arity = ArgumentList();

	uint8_t index = SafeAddConstant(name);

	
	if (arity != r->GetArity()) error(UNDEFINED_RUNNABLE, "Rat '" + name.GetLexeme() + "' takes " + std::to_string(r->GetArity()) 
		+ " arguments, but " + std::to_string(arity) + " were passed", name);

	EmitBytes(OP_CALL, index);
}


void Compiler::unary(bool CanAssign) {
	Token op = advance();

	ParsePrecedence(PREC_UNARY);
	
	switch (op.GetType()) {
		case MINUS:	EmitByte(OP_NEGATE); break;
		case BANG:  EmitByte(OP_NOT);	 break;

	default: break;
	}

}

void Compiler::binary(bool CanAssign) {
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

void Compiler::grouping(bool CanAssign) {
	advance(); // consume left parenthese
	expression(CanAssign);  // irrelevant parameter - expression will allow assigning
	consume(RIGHT_PAREN, (std::string)"Expected ')' after expression");
}

void Compiler::expression(bool CanAssign){
	ParsePrecedence(PREC_ASSIGN);
}

void Compiler::declaration(bool CanAssign) {
	Token kw = Current();
	switch (kw.GetType()) {
		case RAT: {
			advance();
			VarDeclaration();
			break;
		}

		case RUNNABLE: {
			advance();
			RunnableDeclaration();
			break;
		}

		default: statement(CanAssign);
	}
}

void Compiler::statement(bool CanAssign) {
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
	expression(true);  // 'expression' will always allow assignment, regardless of parameter
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


	if (match(EQUALS)) {
		advance();
		expression(true); // expression will always allow assignment, regardless of parameter
	}
	else {
		EmitByte(OP_NONE);
	}

	uint8_t IdIndex;
	if (ct == COMPILE_SCRIPT) {
		IdIndex = SafeAddConstant(identifier);
	}
	else if (ct == COMPILE_RUNNABLE) {
		IdIndex = AddLocal(identifier);
	}

	if (ct == COMPILE_SCRIPT) {
		EmitBytes(OP_DEFINE_GLOBAL, IdIndex);
	}
}


void Compiler::IfStatement() {
	expression(true);	// the condition for the block
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
	short Loopstart = CurrentChunk()->GetSize() - 1;
	expression(true);  // loop condition
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
	expression(true);
	consume(COLON, "Expected ':' after expression");

	EmitByte(OP_REPEAT);

	short Loopstart = CurrentChunk()->GetSize() - 1;

	uint8_t BlockCode = block();
	switch (BlockCode) {
		case BREAK_REPEAT:	advance(); break;

		case UNCLOSED_BLOCK:	ErrorAtCurrent(UNCLOSED_BLOCK, "expected 'endrepeat'");

		default:	ErrorAtCurrent(UNEXPECTED_TOKEN, "expected 'endrepeat'");
	}
	
	EmitByte(OP_END_REPEAT); // will decrement loop value, and jump over the loop instruction if done

	PatchLoop(Loopstart); // will jump to start of loop
}


void Compiler::RunnableDeclaration() {
	if (!match(IDENTIFIER)) ErrorAtCurrent(UNEXPECTED_TOKEN, "Expected function name");

	Token identifier = advance();

	consume(LEFT_PAREN, "Expected '(' after function name");
	std::vector<std::string> args = ParameterList();
	consume(COLON, "Expected ':' after function declaration");

	CurrentBody = new RunnableValue(CurrentBody, new Chunk, args, identifier.GetLexeme());
	this->ct = COMPILE_RUNNABLE;

	uint8_t BlockCode = block();
	switch (BlockCode)
	{
		case BREAK_RUNNABLE:	EmitBytes(OP_NONE, OP_RETURN);	 break;
		case UNCLOSED_BLOCK:	ErrorAtCurrent(UNCLOSED_BLOCK, "Expected 'endrunnable'");

		default:	ErrorAtCurrent(UNEXPECTED_TOKEN, "Expected 'endrunnable'");
	}


	advance();

	RunnableValue* rv = CurrentBody;
	Value v = Value(rv);

	CurrentBody = CurrentBody->GetEnclosing();
	this->ct = COMPILE_SCRIPT;

	uint8_t index = SafeAddConstant(rv);
	EmitBytes(OP_DEFINE_RUNNABLE, index);
}


uint8_t Compiler::ArgumentList() {
	uint8_t ArgCount = 0;
	while (!match(RIGHT_PAREN) && !match(TOKEN_EOF)) {
		expression(true);
		ArgCount++;
		if (match(COMMA)) advance();
	}

	consume(RIGHT_PAREN, "Expected ')' after argument list");

	return ArgCount;
}

std::vector<std::string> Compiler::ParameterList() {
	std::vector<std::string> args;
	
	Token name = peek(-2);

	while (!match(RIGHT_PAREN) && !match(TOKEN_EOF)) {
		consume(IDENTIFIER, "Expected parameter name");
		args.push_back(peek(-1).GetLexeme());
		if (match(COMMA)) advance();
	}

	if (args.size() > 255) error(TABLE_OVERFLOW, "Runnable cannot have over 255 parameters", name);
	consume(RIGHT_PAREN, "Expected ')' after argument list");

	return args;
}


uint8_t Compiler::SafeAddConstant(Token constant){
	// adding a constant to the chunk, wrapped in a try-catch block
	uint8_t index;
	try {
		index = CurrentBody->GetChunk()->AddConstant(constant);
	}
	catch (std::string e) {
		if (e == "Constants overflow")	ErrorAtCurrent(TABLE_OVERFLOW, "Constants overflow - too many constants in a script/runnable");
		if (e == "Float overflow")		ErrorAtCurrent(FLOAT_OVERFLOW, "Value too large - can't be represented as a number value");
	}

	return index;
}

uint8_t Compiler::SafeAddConstant(Value v) {
	// objects that have to be defined as values before insertion
	uint8_t index;
	try {
		index = CurrentBody->GetChunk()->AddConstant(v);
	}
	catch (std::string e) {
		if (e == "Constants overflow")	ErrorAtCurrent(TABLE_OVERFLOW, "Constants overflow - too many constants in a script/runnable");
		if (e == "Float overflow")		ErrorAtCurrent(FLOAT_OVERFLOW, "Value too large - can't be represented as a number value");
	}

	return index;
}


uint8_t Compiler::AddLocal(Token Identifier) {
	if (this->CurrentBody->GetLocals().size() >= 255) {
		ErrorAtPrevious(TABLE_OVERFLOW, "Too many local variables in a runnable");
	}

	return this->CurrentBody->AddLocal(Identifier.GetLexeme());
}

short Compiler::ResolveLocal(Token Identifier) {
	return this->CurrentBody->ResolveLocal(Identifier.GetLexeme());
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

	bool CanAssign = (precedence <= PREC_ASSIGN);
	(this->*PrefixRule)(CanAssign);


	while (precedence <= GetRule(Current().GetType()).precedence) {

		rule = GetRule(Current().GetType());
		ParseFunction InfixRule = rule.infix;
		if (InfixRule == nullptr) {
			ErrorAtCurrent(UNEXPECTED_TOKEN, "Expected expression");
		}
		(this->*InfixRule)(CanAssign);
	}
	if (match(EQUALS)) { // token hasn't been consumed, meaning assignment target was invalid
		ErrorAtCurrent(UNEXPECTED_TOKEN, "Invalid assignment target");
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

Chunk *Compiler::CurrentChunk() {
	return CurrentBody->GetChunk();
}

void Compiler::EmitByte(uint8_t byte) {
	CurrentChunk()->Append(byte);
}

void Compiler::EmitBytes(uint8_t byte1, uint8_t byte2) {
	CurrentChunk()->Append(byte1, byte2);
}

void Compiler::EmitReturn() {
	CurrentChunk()->Append(OP_RETURN);
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
			index = CurrentChunk()->GetSize() - 1;
			return index;

		default:
			ErrorAtCurrent(INTERNAL_ERROR, "");
			break;
	}
}

void Compiler::PatchJump(short JumpIndex) {
	short CurrentIndex = CurrentChunk()->GetSize() - 1;
	if (CurrentIndex < JumpIndex) ErrorAtCurrent(INTERNAL_ERROR, "");

	CurrentChunk()->PatchJump(JumpIndex, CurrentIndex - JumpIndex);
}

void Compiler::PatchLoop(short LoopStart) {
	EmitByte(OP_LOOP);
	EmitBytes(0, 0);

	short CurrentIndex = CurrentChunk()->GetSize() - 1;
	if (CurrentIndex < LoopStart) ErrorAtCurrent(INTERNAL_ERROR, "");

	CurrentChunk()->PatchJump(CurrentIndex, CurrentIndex - LoopStart);
}