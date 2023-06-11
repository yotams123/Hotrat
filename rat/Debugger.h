#pragma once

#include "Chunk.h"
#include "Interpreter.h"

#include <iostream>
#include <iomanip>

class Debugger 
{
private:
	Chunk *chunk;
	int offset;
	std::string ChunkName;
	std::vector<uint8_t> code;

	int line;
	std::string PrintLineNum;

	void ConstantOperation(const std::string& name);
	void SimpleOperation(const std::string& name);
	void JumpOperation(const std::string& name);
	void CallOperation(const std::string& name);
	void RunnableDefinition(const std::string& name);
	
	void DisassembleInstruction();

public:
	Debugger(Chunk *, std::string);
	~Debugger();

	void DisassembleChunk();
};
