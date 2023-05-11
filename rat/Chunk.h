#pragma once

#include <iostream>
#include <vector>

#include "Token.h"

typedef struct Chunk {
private:

	std::vector<uint8_t> code;
	std::vector<uint8_t>::iterator ip;

	std::vector<int> constants;
	struct Chunk* enclosing;

public:
	Chunk();
	uint8_t AddConstant(Token);

	void Append(uint8_t);
	void Append(uint8_t, uint8_t);

	uint8_t advance();
	int ReadConstant(uint8_t index);

	std::vector<uint8_t>& GetCode();
	bool IsAtEnd();

	void SyncIP();
} Chunk;

