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
	std::transform(std::begin(original), std::end(original), std::back_inserter(original),
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
	if (iter == std::end(token_line)) {
		throw std::invalid_argument("no command provided");
	}
	++iter;
	if (iter == std::end(token_line)) {
		return{};
	}
	if (iter->category != Category::LeftParenthesis) {
		throw std::invalid_argument("expected left parenthesis after command");
	}
	++iter;
	if (iter == std::end(token_line)) {
		throw std::invalid_argument("unmatched left parenthesis");
	}
	std::string argument{};
	// in python terminal, you can use numbers as arguments. for our purposes, only identifiers and strings are allowed

	// collect everything until close parenthesis. make it so .value always has something.

	if (iter->category != Category::RightParenthesis && iter->category != Category::Identifier && iter) {
		throw std::invalid_argument("expected identifier in command argument");
	}
	if (iter->category == Category::Identifier){
		argument = std::get<std::string>(iter->value);
		++iter;
	}
	if (iter->category != Category::RightParenthesis) {
		throw std::invalid_argument("expected right parenthesis after command argument");
	}

	
	++iter;
	if (iter == std::end(token_line)) {
		throw std::invalid_argument("unmatched left parenthesis");
	}
	if (iter->category != Category::LeftParenthesis) {
		throw std::invalid_argument("expected right parenthesis after command argument");
	}

	//first is command
	//second is left paren (or nothing)
	//third is the argument (as an identifier)
	//fourth is close paren
	// fifth is nothing
}