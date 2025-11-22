#include "lexical_analyzer.hpp"
#include "input_parser.hpp"

#include <charconv>
#include <stdexcept>

std::vector<Token> LexicalAnalyzer::GenerateTokens(std::string_view input_string) {
	std::vector<Token> tokens{};

	for (auto curr_char = std::begin(input_string); curr_char != std::end(input_string);) {
		Token new_token{};

		if (std::isdigit(*curr_char)) {
			// numeric literal
			new_token.category = Category::NumericLiteral;
				
			auto start_char = curr_char;
			while (curr_char != std::end(input_string) && !InputParser::IsNewTokenChar(*curr_char)) {
				++curr_char;
			}

			// convert to int
			int new_number{};
			size_t start_idx = start_char - std::begin(input_string);
			size_t end_idx = start_idx + (curr_char - start_char);
			const char* first = input_string.data() + start_idx;
			const char* last = input_string.data() + end_idx;
			auto conversion_result = std::from_chars(first, last, new_number);
			if (conversion_result.ptr != last) {
				// we don't use std::errc::invalid_argument because it is only called if *no pattern was matched*
				throw std::invalid_argument("invalid numeric literal");
			}
			if (conversion_result.ec == std::errc::result_out_of_range) {
				throw std::out_of_range("numeric literal is out of range");
			}
			new_token.value = new_number;
		}
		else if (std::isalpha(*curr_char) || *curr_char == '_') {
			// identifier, keyword, or logical operator
			// '_' is valid for Python. numbers are valid if they are not first char.
			while (curr_char != std::end(input_string) && !InputParser::IsNewTokenChar(*curr_char)) {
				if (!std::isalnum(*curr_char) && *curr_char != '_') {
					throw std::invalid_argument("invalid identifier");
				}
				std::get<std::string>(new_token.value) += *curr_char;
				++curr_char;
			}

			if (InputParser::IsNonLogicalOperatorKeyword(std::get<std::string>(new_token.value))) {
				new_token.category = Category::Keyword;
			}
			else if (InputParser::IsLogicalOperatorKeyword(std::get<std::string>(new_token.value))) {
				new_token.category = Category::LogicalOperator;
			}
			else {
				new_token.category = Category::Identifier;
			}
		}
		else if (*curr_char == '\'' || *curr_char == '\"') {
			new_token.category = Category::StringLiteral;
			auto open_quote = curr_char;
			++curr_char;	// skip open quote
			while (curr_char != std::end(input_string) && *curr_char != *open_quote) {
				std::get<std::string>(new_token.value) += *curr_char;
				++curr_char;
			}
			if (curr_char == std::end(input_string)) {
				throw std::invalid_argument("unterminated string literal");
			}
			++curr_char;	// skip close quote
		}
		else if (*curr_char == '#') {
			new_token.category = Category::Comment;
			++curr_char;	// skip number sign
				
			new_token.value = std::string{ input_string.substr(curr_char - std::begin(input_string), std::end(input_string) - curr_char) };
			curr_char = std::end(input_string);
		}
		else if (*curr_char == '(') {
			new_token.category = Category::LeftParenthesis;
			++curr_char;
		}
		else if (*curr_char == ')') {
			new_token.category = Category::RightParenthesis;
			++curr_char;
		}
		else if (*curr_char == ':') {
			new_token.category = Category::Colon;
			++curr_char;
		}
		else if (*curr_char == ',') {
			new_token.category = Category::Comma;
			++curr_char;
		}
		else if (InputParser::IsRelationalOrAssignmentOperator(*curr_char)) {
			new_token.value = std::string{ *curr_char };
			++curr_char;
			// second operator, if any, can only be '='
			if (curr_char != std::end(input_string) && *curr_char == '=') {
				std::get<std::string>(new_token.value) += *curr_char;
				++curr_char;
			}
			if (std::get<std::string>(new_token.value) == "=") {
				new_token.value = "";	// erase string data (it's redundant given the enum)
				new_token.category = Category::AssignmentOperator;
			}
			else if (std::get<std::string>(new_token.value) == "!") {
				throw std::invalid_argument("invalid \'!\' operator");
			}
			else {
				new_token.category = Category::RelationalOperator;
			}
		}
		else if (InputParser::IsArithmeticOperator(*curr_char)) {
			new_token.category = Category::ArithmeticOperator;
			new_token.value = std::string{ *curr_char };
			++curr_char;
		}
		else if (InputParser::IsWhitespace(*curr_char)) {
			if (curr_char != std::begin(input_string)) {
				++curr_char;
				continue;	// skip insertion
			}
			new_token.category = Category::Indent;
			++curr_char;
		}
		else if (*curr_char == '\n') {
			new_token.category = Category::Newline;
			++curr_char;
		}
		else {
			throw std::invalid_argument("invalid character");
		}

		// insert token
		tokens.push_back(new_token);
	}
	return tokens;
}