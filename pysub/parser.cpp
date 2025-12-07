#include "parser.hpp"

bool Parser::Match(auto&&... input) {
	bool input_matches = (Check(input) && ... && true);
	if (input_matches) {
		IncrementToken();
	}
	return input_matches;
}