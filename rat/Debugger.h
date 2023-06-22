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

	std::vector<std::pair<int, int>> runnables;
	int line;
	std::string PrintLineNum;

	void ConstantOperation(const std::string& name);
	void SimpleOperation(const std::string& name);
	void JumpOperation(const std::string& name);
	void CallNativeOperation(const std::string& name);
	void RunnableDefinition(const std::string& name);
	
	void DisassembleInstruction();

	void DisassembleRunnable(RunnableValue* runnable, int line);

public:
	Debugger(Chunk *, std::string);
	~Debugger();

	void DisassembleScript();
};
