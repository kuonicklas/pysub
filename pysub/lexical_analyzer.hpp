#ifndef LEXICAL_ANALYZER_HPP
#define LEXICAL_ANALYZER_HPP

#include "globals.hpp"

class LexicalAnalyzer {
public:
	static std::vector<Token> GenerateTokens(std::string_view input_string);
};

#endif