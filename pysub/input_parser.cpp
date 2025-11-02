#include "input_parser.hpp"

#include <iterator>
#include <algorithm>

std::string InputParser::ToLowerCase(std::string_view original) {
	std::string lowercase{};
	std::transform(std::begin(original), std::end(original), std::back_inserter(lowercase),
		[](char c) {return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
	return lowercase;
}