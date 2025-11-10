#ifndef INPUT_PARSER_HPP
#define INPUT_PARSER_HPP

#include <string>

class InputParser {
public:
	static std::string ToLowerCase(std::string_view string);
	static void ToLowerCase(std::string& string);
	static void TrimLeadingAndTrailingWhitespaces(std::string& string);
	static bool IsWhitespace(char c);
	static bool IsSymbol(char c);
	static bool IsOperator(char c);
	static bool IsNewTokenChar(char c);
	static bool IsNonLogicalOperatorKeyword(std::string_view string);
	static bool IsLogicalOperatorKeyword(std::string_view string);
	static bool IsRelationalOrAssignmentOperator(char c);
	static bool IsArithmeticOperator(char c);
};

#endif