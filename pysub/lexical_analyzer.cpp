#include "lexical_analyzer.hpp"
#include "input_parser.hpp"

#include <charconv>
#include <stdexcept>

TokenLine LexicalAnalyzer::GenerateTokens(std::string input_code) {
	// we pass by copy and move
	return GenerateTokens(std::vector<std::string>{ std::move(input_code)}).front();
}

std::vector<TokenLine> LexicalAnalyzer::GenerateTokens(const std::vector<std::string>& input_code) {
	std::vector<TokenLine> token_lines{};
	token_lines.reserve(input_code.size());

	for (const auto& input_line : input_code) {
		token_lines.push_back({});
		auto curr_token_line = --std::end(token_lines);
		auto curr_char = std::begin(input_line);

		// get tokens
		while (curr_char != std::end(input_line)) {
			Token new_token{};

			// construct token
			if (std::isdigit(*curr_char)) {
				// numeric literal
				new_token.category = Category::NumericLiteral;
				
				auto start_char = curr_char;
				while (curr_char != std::end(input_line) && !InputParser::IsNewTokenChar(*curr_char)) {
					++curr_char;
				}

				// convert to int
				int new_number{};
				size_t start_idx = start_char - std::begin(input_line);
				size_t end_idx = start_idx + (curr_char - start_char);
				const char* first = input_line.data() + start_idx;
				const char* last = input_line.data() + end_idx;
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
				while (curr_char != std::end(input_line) && !InputParser::IsNewTokenChar(*curr_char)) {
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
				while (curr_char != std::end(input_line) && *curr_char != *open_quote) {
					std::get<std::string>(new_token.value) += *curr_char;
					++curr_char;
				}
				if (curr_char == std::end(input_line)) {
					throw std::invalid_argument("unterminated string literal");
				}
				++curr_char;	// skip close quote
			}
			else if (*curr_char == '#') {
				new_token.category = Category::Comment;
				++curr_char;	// skip number sign
				
				new_token.value = input_line.substr(curr_char - std::begin(input_line), std::end(input_line) - curr_char);
				curr_char = std::end(input_line);
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
				if (curr_char != std::end(input_line) && *curr_char == '=') {
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
				if (curr_char != std::begin(input_line)) {
					++curr_char;
					continue;	// skip insertion
				}
				new_token.category = Category::Indent;
				++curr_char;
			}
			else {
				throw std::invalid_argument("invalid character");
			}

			// insert token
			curr_token_line->push_back(new_token);
		}
	}
	return token_lines;
}