#ifndef PARSER_HPP
#define PARSER_HPP

#include "globals.hpp"

#include <vector>

// credit Robert Nystrom's "Crafting Interpreters" book for the basic structure:
// https://craftinginterpreters.com/
class Parser {
public:
	explicit Parser(const std::vector<Token>& _tokens) : tokens(_tokens){
		curr_token = std::cbegin(tokens);
	}
private:
	const std::vector<Token>& tokens;
	std::vector<Token>::const_iterator curr_token;

	bool Match(auto&&... input);
	void IncrementToken() {
		if (curr_token != std::end(tokens)) {
			++curr_token;
		}
	}
	bool Check(std::string_view s) const;
	bool Check(Category category) const;
};

#endif