#include "input_parser.hpp"

#include <iterator>
#include <algorithm>

std::string InputParser::ToLowerCase(std::string_view original) {
	std::string lowercase{};
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