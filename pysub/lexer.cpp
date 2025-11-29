#include "lexer.hpp"

#include <charconv>
#include <stdexcept>
#include <cassert>

std::vector<Token> Lexer::GenerateTokens(std::string_view input_string) {
	std::vector<Token> tokens{};
	//int curr_indent_num = 0;
	
	auto IsEndOfFile = [&input_string](const auto iter) {return iter == std::end(input_string) || *iter == '\0'; };
	auto IsEndOfLine = [&](const auto iter) {return IsEndOfFile(iter) || *iter == '\n'; };
	auto IsStartOfNewLine = [&]() {return tokens.empty() || tokens.back().category == Category::Newline; };
	int indent_level = 0;

	for (auto curr_char = std::begin(input_string); !IsEndOfFile(curr_char);) {
		Token new_token{};

		// check indentation
		if (IsStartOfNewLine()) {
			// note: dedents occur *after* a newline, not before. a block expects a dedent *after statement(s)*, and a statement is defined as ending with a newline (or eof).
			int curr_indentation = 0;
			const auto char_start = curr_char;
			while (!IsEndOfLine(curr_char) && Utilities::IsWhitespace(*curr_char)) {
				++curr_indentation;
				++curr_char;
			}
			for (int i = 0; i < std::abs(curr_indentation - indent_level); ++i) {
				Token new_indent_token{};
				if (curr_indentation > indent_level) {
					new_indent_token.category = Category::Indent;
				}
				else {
					new_indent_token.category = Category::Dedent;
				}
				tokens.push_back(new_indent_token);
			}
			indent_level = curr_indentation;
			if (char_start != curr_char) {
				continue;	// stream has advanced; recheck loop condition
			}
		}

		if (std::isdigit(*curr_char)) {
			// numeric literal
			new_token.category = Category::NumericLiteral;
				
			auto start_char = curr_char;
			while (!IsEndOfLine(curr_char) && !Utilities::IsNewTokenChar(*curr_char)) {
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
			while (!IsEndOfLine(curr_char) && !Utilities::IsNewTokenChar(*curr_char)) {
				if (!std::isalnum(*curr_char) && *curr_char != '_') {
					throw std::invalid_argument("invalid identifier");
				}
				std::get<std::string>(new_token.value) += *curr_char;
				++curr_char;
			}

			if (Utilities::IsNonLogicalOperatorKeyword(std::get<std::string>(new_token.value))) {
				new_token.category = Category::Keyword;
			}
			else if (Utilities::IsLogicalOperatorKeyword(std::get<std::string>(new_token.value))) {
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
			while (!IsEndOfLine(curr_char) && *curr_char != *open_quote) {
				std::get<std::string>(new_token.value) += *curr_char;
				++curr_char;
			}
			if (IsEndOfLine(curr_char)) {
				throw std::invalid_argument("unterminated string literal");
			}
			++curr_char;	// skip close quote
		}
		else if (*curr_char == '#') {
			new_token.category = Category::Comment;
			++curr_char;	// skip number sign
			
			const auto comment_start = curr_char;
			while (!IsEndOfLine(curr_char)) {
				++curr_char;
			}
			new_token.value = std::string{ input_string.substr(comment_start - std::begin(input_string), curr_char - comment_start)};
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
		else if (Utilities::IsRelationalOrAssignmentOperator(*curr_char)) {
			new_token.value = std::string{ *curr_char };
			++curr_char;
			// second operator, if any, can only be '='
			if (!IsEndOfLine(curr_char) && *curr_char == '=') {
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
		else if (Utilities::IsArithmeticOperator(*curr_char)) {
			new_token.category = Category::ArithmeticOperator;
			new_token.value = std::string{ *curr_char };
			++curr_char;
		}
		else if (Utilities::IsWhitespace(*curr_char)) {
			assert(!IsStartOfNewLine() && "discarding whitespaces occurs after indentation is assumed to be checked first");
			++curr_char;
			continue;	// skip insertion
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
	
	//// append dedents
	//// note: if EOF is implicitly treated as dedent(s), then no need for this.
	//for (int i = indent_level; i > 0; --i) {
	//	tokens.push_back(Token{ .category = Category::Dedent });
	//}

	return tokens;
}