#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "Token.h"


class Scanner {
private:
	std::string src;
	std::vector<Token> tokens;

	int start;
	int current;
	short line;

public:
	Scanner(std::string src);
	std::vector<Token> ScanTokens();

private:
	Token ScanToken();

	char advance();
	bool match(char c);
	char peek(int level);

	char GetCurrentChar();

	void SkipWhiteSpace();
	bool CheckWord(std::string word);

	Token String();

	Token Number();

	Token Identifier();

	void error(std::string ErrorMsg);
	int CountLines();
};