#include "Chunk.h"

#include <limits>

Chunk::Chunk() {
	ip = 0;

	constants = std::vector<Value*>();
	enclosing = nullptr;
}


Chunk::Chunk(Chunk *enclosing) {
	this->ip = 0;

	this->constants = std::vector<Value*>();
	this->enclosing = enclosing;
}

Chunk::Chunk(Chunk *code, Chunk* enclosing) {
	this->ip = 0;

	this->constants = code->constants;
	this->enclosing = enclosing;

	this->code = code->GetCode();
}

Chunk::~Chunk() {
	std::vector<Value*>::iterator i;
	for (i = constants.begin(); i < constants.end(); i++)
	{
		delete* i;
	}
}

uint8_t Chunk::advance() {
	return code[ip++];
}

std::vector<Value*>& Chunk::GetConstants() {
	return this->constants;
}


uint8_t Chunk::AddConstant(Token constant) {

	if (constants.size() >= 256) {
		throw std::string("Constants overflow");
	}

	Value *val;

	TokenType t = constant.GetType();
	switch (t) {
		case NUM_LITERAL: {
			try {
				val = new NumValue(std::stof(constant.GetLexeme()));	break;
			}
			catch (const std::exception& e) {
				throw std::string("Float overflow");
			}
			break;
		}

		case TRUE:			val = new BoolValue(true); break;
		case FALSE:			val = new BoolValue(false); break;

		case STRING_LITERAL:
		case IDENTIFIER:	val = new StrValue((std::string&)constant.GetLexeme()); break;
	}

	constants.push_back(val);
	return (uint8_t)(constants.size() - 1); // index of constant
}

uint8_t Chunk::AddConstant(Chunk *ByteCode, uint8_t arity, std::string name) {

	if (constants.size() >= 256) {
		throw std::string("Constants overflow");
	}

	RunnableValue* val = new RunnableValue(ByteCode, arity, name);

	constants.push_back(val);
	return (uint8_t)(constants.size() - 1); // index of constant
}



Value *Chunk::ReadConstant(uint8_t index) {
	return constants[index];
}


void Chunk::ClearConstants() {
	for (int i = 0; i < this->constants.size(); i++) {
		try {
			delete this->constants[i];
			this->constants[i] = nullptr;
		}
		catch (const std::exception& e) {
			continue;
		}
	}
}

void Chunk::Append(uint8_t byte) {
	code.push_back(byte);
}

void Chunk::Append(uint8_t byte1, uint8_t byte2) {
	code.push_back(byte1);
	code.push_back(byte2);
}

std::vector<uint8_t>& Chunk::GetCode() {
	return this->code;
}

bool Chunk::IsAtEnd() {
	return this->ip > (this->code.size() - 1);
}

int Chunk::CountLines() {
	int line = 1;
	short op = 0;
	while (op < ip) {
		switch (this->code[op]) {
			case OP_CONSTANT:
			case OP_DEFINE_GLOBAL:
			case OP_GET_GLOBAL:
			case OP_SET_GLOBAL:

			case OP_INC:
			case OP_DEC: {
				op++;
				op++;  // skip over opcode and operand
				break;
			}
			
			case OP_NEWLINE:	line++;
			default:	op++;
		}
	}
	return line;
}

short Chunk::GetOffset() {
	return this->ip;
}

short Chunk::GetSize() {
	return this->code.size();
}


Chunk* Chunk::GetEnclosing() {
	return this->enclosing;
}

void Chunk::MoveIp(short distance) {
	ip += distance;
}

void Chunk::PatchJump(short JumpIndex, short distance) {
	this->code[JumpIndex - 1] = (uint8_t)((distance >> 8) & 0xFF);
	this->code[JumpIndex] = (uint8_t)(distance & 0xFF);
}