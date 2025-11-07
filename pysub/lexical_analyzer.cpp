#include "lexical_analyzer.hpp"

#include <charconv>

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

		// get tokens for the line
		while (curr_char != std::end(input_line)) {
			Token new_token{};

			if (std::isdigit(*curr_char)) {
				new_token.category = Category::NumericLiteral;
				
				auto start_char = curr_char;
				while (curr_char != std::end(input_line) && std::isdigit(*curr_char)) {
					++curr_char;
				}

				// convert to int
				int new_number{};
				size_t start_idx = start_char - std::begin(input_line);
				size_t end_idx = start_idx + (curr_char - start_char);
				auto conversion_result = std::from_chars(input_line.data() + start_idx, input_line.data() + end_idx, new_number);
				if (conversion_result.ec == std::errc::invalid_argument) {
					throw std::exception("invalid numeric literal");
				}
				if (conversion_result.ec == std::errc::result_out_of_range) {
					throw std::exception("numeric literal is out of range");
				}
			}
			else if (std::isalpha(*curr_char) || *curr_char == '_') {
				new_token.category = Category::Identifier; //Could be a keyword or logical operator; this is checked later.

				//'_' is valid for Python. Numbers are valid if they are not first char.
				while (iter != inputLine.end()) {
					if (isalnum(*iter) || (*iter == '_')) {
						copyPair.first += *iter; //Insert chars
						iter++;
					}
					else break;
				}

				//Check if it's a keyword or logical operator
				if ((copyPair.first == "print") || (copyPair.first == "if") || (copyPair.first == "elif") ||
					(copyPair.first == "else") || (copyPair.first == "while") || (copyPair.first == "int") ||
					(copyPair.first == "input")) {

					copyPair.second = LexicalAnalyzer::categoryType::KEYWORD;
				}
				else if ((copyPair.first == "and") || (copyPair.first == "or") || (copyPair.first == "not")) {
					copyPair.second = LexicalAnalyzer::categoryType::LOGICAL_OP;
				}
			}
			else {
				switch (*iter) {
				case '\'':
				case '\"':
				{
					copyPair.second = LexicalAnalyzer::categoryType::STRING_LITERAL;

					//Insert first quote
					copyPair.first += *iter;
					iter++;

					//Insert until quote reached or end reached
					bool closedQuoteFound = false;

					while (iter != inputLine.end()) {
						copyPair.first += *iter;

						if ((*iter == '\'') || (*iter == '\"')) {
							closedQuoteFound = true;
							iter++;
							break;
						}
						iter++;
					}

					//If opening/closing quotes don't match, or not found, then categorize as unknown
					if ((*copyPair.first.begin() != *copyPair.first.rbegin()) && (closedQuoteFound))
						throw string("Opening and closing quotes don't match!");
					else if (!closedQuoteFound)
						throw string("Closing quotes not found!");
				}
				break;
				case '(':
					copyPair.second = LexicalAnalyzer::categoryType::LEFT_PAREN;

					copyPair.first += *iter; //Insert into pair
					iter++;
					break;
				case ')':
					copyPair.second = LexicalAnalyzer::categoryType::RIGHT_PAREN;

					copyPair.first += *iter; //Insert into pair
					iter++;
					break;
				case ':':
					copyPair.second = LexicalAnalyzer::categoryType::COLON;

					copyPair.first += *iter; //Insert into pair
					iter++;
					break;
				case ',':
					copyPair.second = LexicalAnalyzer::categoryType::COMMA;

					copyPair.first += *iter; //Insert into pair
					iter++;
					break;
				case '#':
					copyPair.second = LexicalAnalyzer::categoryType::COMMENT;

					copyPair.first = inputLine.substr(distance(inputLine.begin(), iter), distance(iter, inputLine.end()));
					iter = inputLine.end(); //Jump to end
					break;
				case ' ':
				case'\t':
					copyPair.second = LexicalAnalyzer::categoryType::INDENT;

					if (iter == inputLine.begin()) {
						copyPair.first += *iter; //Insert as indent
					}
					iter++;
					break; //If not in beginning, skip insertion
				case '=':
				case '<':
				case '>':
				case '!':
					copyPair.second = LexicalAnalyzer::categoryType::RELATIONAL_OP; //Could be an assignment op; this is checked later.

					copyPair.first += *iter; //Insert first operator
					iter++;

					//Check for second operator (which can only be '=')
					if ((iter != inputLine.end()) && (*iter == '=')) {
						copyPair.first += *iter;
						iter++;
					}

					//Check if it is only an assignment operator
					if (copyPair.first == "=") {
						copyPair.second = LexicalAnalyzer::categoryType::ASSIGNMENT_OP;
					}
					else if (copyPair.first == "!") {
						//Exclamation mark by itself is unknown
						copyPair.second = LexicalAnalyzer::categoryType::UNKNOWN;
					}

					break;
				case '+':
				case '-':
				case '*':
				case '/':
				case '%':
					copyPair.second = LexicalAnalyzer::categoryType::ARITH_OP;

					copyPair.first += *iter; //Insert
					iter++;

					break;
				default:
					copyPair.second = LexicalAnalyzer::categoryType::UNKNOWN;

					copyPair.first += *iter; //Insert
					iter++;
				}
			}

			// note: push back assuming that we break the iteration if no token was produced
			curr_token_line->push_back(new_token);

			if (copyPair.first != "") {
				copyLine.push_back(copyPair); //Push copy of pair into result vector (if something was inserted in the string)
			}

			copyPair.first.clear(); //Clear string for next pair
		}

		resultToken.push_back(copyLine); //Push line into result
		copyLine.clear(); //Clear vector before analyzing next line
	}

	return resultToken;
}