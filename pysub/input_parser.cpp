#include "input_parser.hpp"

#include <iterator>
#include <algorithm>
#include <stdexcept>

std::string InputParser::ToLowerCase(std::string_view original) {
	std::string lowercase{};
	lowercase.reserve(original.size());
	std::transform(std::begin(original), std::end(original), std::back_inserter(lowercase),
		[](char c) {return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
	return lowercase;
}

void InputParser::ToLowerCase(std::string& original) {
	std::transform(std::begin(original), std::end(original), std::begin(original),
		[](char c) {return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
}

void InputParser::TrimLeadingAndTrailingWhitespaces(std::string& string) {
	size_t start = string.find_first_not_of(" \t");
	if (start == std::string::npos) {
		return;
	}
	size_t end = string.find_last_not_of(" \t");
	string = string.substr(start, end - start + 1);
}

bool InputParser::IsWhitespace(char c) {
	return c == ' ' || c == '\t';
}

bool InputParser::IsSymbol(char c) {
	return c == '\'' || c == '\"' || c == '(' || c == ')' || c == ':' || c == ', ' || c == '#';
}

bool InputParser::IsOperator(char c) {
	return IsRelationalOrAssignmentOperator(c) || IsArithmeticOperator(c);
}

bool InputParser::IsNewTokenChar(char c) {
	// returns true if the char is the beginning of a new token/end of the current one (assuming it is not part of a string literal or comment)
	return IsWhitespace(c) || IsSymbol(c) || IsOperator(c);
}

bool InputParser::IsNonLogicalOperatorKeyword(std::string_view string) {
	return string == "print" || string == "if" || string == "elif" || string == "else" || string == "while" || string == "int" || string == "input";
}

bool InputParser::IsLogicalOperatorKeyword(std::string_view string) {
	return string == "and" || string == "or" || string == "not";
}

bool InputParser::IsRelationalOrAssignmentOperator(char c) {
	return c == '=' || c == '<' || c == '>' || c == '!';
}

bool InputParser::IsArithmeticOperator(char c) {
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '%';
}

std::optional<std::string> InputParser::GetCommandArgument(const TokenLine& token_line) {
	auto iter = std::begin(token_line);

	// command
	if (iter == std::end(token_line)) {
		throw std::invalid_argument("no command provided");
	}
	++iter;

	// left parenthesis (or nothing)
	if (iter == std::end(token_line)) {
		return{};
	}
	if (iter->category != Category::LeftParenthesis) {
		throw std::invalid_argument("expected left parenthesis after command");
	}
	++iter;

	// (argument), right parenthesis
	if (iter == std::end(token_line)) {
		throw std::invalid_argument("unmatched left parenthesis");
	}
	// in python terminal, you can use expressions as arguments (its type becomes the argument). for our purposes, only identifiers and strings are allowed
	std::string argument{};
	if (iter->category != Category::RightParenthesis && iter->category != Category::Identifier && iter->category != Category::StringLiteral) {
		throw std::invalid_argument("expected identifier or string literal in command argument");
	}
	if (iter->category != Category::RightParenthesis){
		argument = std::get<std::string>(iter->value);
		++iter;
	}
	if (iter->category != Category::RightParenthesis) {
		throw std::invalid_argument("expected right parenthesis after command argument");
	}
	++iter;

	// (comment), end
	if (iter != std::end(token_line) && iter->category == Category::Comment) {
		++iter;
	}	
	if (iter != std::end(token_line)) {
		throw std::invalid_argument("invalid syntax after command");
	}
	return argument;
}