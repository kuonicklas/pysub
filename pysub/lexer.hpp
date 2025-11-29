#ifndef LEXER_HPP
#define LEXER_HPP

#include "globals.hpp"

class Lexer {
public:
	static std::vector<Token> GenerateTokens(std::string_view input_string);
};

#endif