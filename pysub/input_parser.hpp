#ifndef INPUT_PARSER_HPP
#define INPUT_PARSER_HPP

#include "globals.hpp"

#include <string>
#include <optional>

class InputParser {
public:
	// string processing
	static std::string ToLowerCase(std::string_view string);
	static void ToLowerCase(std::string& string);
	static void TrimLeadingAndTrailingWhitespaces(std::string& string);

	// analysis
	static bool IsWhitespace(char c);
	static bool IsSymbol(char c);
	static bool IsOperator(char c);
	static bool IsNewTokenChar(char c);
	static bool IsNonLogicalOperatorKeyword(std::string_view string);
	static bool IsLogicalOperatorKeyword(std::string_view string);
	static bool IsRelationalOrAssignmentOperator(char c);
	static bool IsArithmeticOperator(char c);

	// parsing
	static std::optional<std::string> GetCommandArgument(const std::vector<Token>& token_line);

	// exceptions
	static std::exception AddContext(const std::string& context, const std::exception& ex);
};

#endif