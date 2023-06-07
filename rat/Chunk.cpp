#include "Chunk.h"

#include <limits>

Chunk::Chunk() {
	ip = 0;

	constants = std::vector<Value>();
}

Chunk::Chunk(Chunk *ToCopy) {
	this->code = ToCopy->code;
	this->constants = ToCopy->constants;

	this->ip = 0;
}

Chunk::~Chunk() {
	this->ClearConstants();
}

uint8_t Chunk::advance() {
	return code[ip++];
}

std::vector<Value>& Chunk::GetConstants() {
	return this->constants;
}


uint8_t Chunk::AddConstant(Token& constant) {

	if (constants.size() >= 256) {
		throw std::string("Constants overflow");
	}

	Value val;

	TokenType t = constant.GetType();
	switch (t) {
		case NUM_LITERAL: {
			try {
				val = Value(std::stof(constant.GetLexeme()));	break;
			}
			catch (const std::exception& e) {
				throw std::string("Float overflow");
			}
			break;
		}

		case TRUE:			val = Value(true); break;
		case FALSE:			val = Value(false); break;

		case STRING_LITERAL:
		case IDENTIFIER: {
			StrValue* o = new StrValue(constant.GetLexeme());
			val = Value(o); break;
		}
	}

	constants.push_back(val);
	return (uint8_t)(constants.size() - 1); // index of constant
}

uint8_t Chunk::AddConstant(Value v) {

	if (constants.size() >= 256) {
		throw std::string("Constants overflow");
	}

	constants.push_back(v);
	return (uint8_t)(constants.size() - 1); // index of constant
}



Value Chunk::ReadConstant(uint8_t index) {
	return constants[index];
}


void Chunk::ClearConstants() {
	for (int i = 0; i < this->constants.size(); i++) {
		try {
			if (this->constants[i].GetType() == Value::OBJECT_T) {
				ObjectValue* o = constants[i].GetObject();
				delete o;
				constants[i].SetValue((ObjectValue*)nullptr);
			}
		}
		catch (const std::exception& e) {
			continue;
		}
	}
}

short Chunk::FindRunnable(Token& identifier) {
	std::string name = identifier.GetLexeme();
	for (int i = 0; i < this->constants.size(); i++) {
		if (constants[i].GetType() == Value::OBJECT_T) {
			ObjectValue* o = constants[i].GetObject();

			if (o->GetType() == ObjectValue::RUNNABLE_T) {
				RunnableValue* r = (RunnableValue *)o;
				if (r->GetName() == name) return i;
			}
		}
	}

	return -1;
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

			case OP_INC_GLOBAL:
			case OP_DEC_GLOBAL: {
				op += 2;
				break;
			}

			case OP_JUMP:
			case OP_JUMP_IF_FALSE:
			case OP_JUMP_IF_TRUE:
			case OP_LOOP: {
				op += 3;
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

void Chunk::MoveIp(short distance) {
	ip += distance;
}

void Chunk::PatchJump(short JumpIndex, short distance) {
	this->code[JumpIndex - 1] = (uint8_t)((distance >> 8) & 0xFF);
	this->code[JumpIndex] = (uint8_t)(distance & 0xFF);
}