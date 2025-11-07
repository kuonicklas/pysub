#ifndef LEXICAL_ANALYZER_HPP
#define LEXICAL_ANALYZER_HPP

#include "globals.hpp"

class LexicalAnalyzer {
public:
	static std::vector<TokenLine> GenerateTokens(const std::vector<std::string>& input_code);
	static TokenLine GenerateTokens(std::string input_code);
};

#endif