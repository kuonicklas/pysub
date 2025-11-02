#ifndef INPUT_PARSER_HPP
#define INPUT_PARSER_HPP

#include <string>

class InputParser {
public:
	static std::string ToLowerCase(std::string_view original);
};

#endif