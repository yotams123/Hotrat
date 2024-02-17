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
	int line;

	bool HadError;

public:
	Scanner(std::string src);
	~Scanner();

	std::vector<Token>& ScanTokens();

private:
	Token ScanToken();

	char advance();
	bool match(char c);
	char peek(int distance);
	bool MatchString(std::string target);

	bool IsAtEnd();

	void SkipWhiteSpace();
	bool CheckWord(std::string word);

	Token String(char quote_type);
	Token Number();
	Token Identifier();
	Token Comment();

	void error(std::string ErrorMsg);
	void error(std::string ErrorMsg, char violator);
};