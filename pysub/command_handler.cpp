#include "command_handler.hpp"

#include <algorithm>

// returns true if any token contains a command
bool CommandHandler::IsCommand(const TokenLine& token_line) const {
	return std::any_of(std::begin(token_line), std::end(token_line), [this](const auto& token) -> bool {
		return token.category == Category::Identifier && this->IsCommand(token.string);
		});
}

bool CommandHandler::IsCommand(const std::string& s) const {
	// string is used instead of string_view to avoid string_view -> string conversion
	return command_list.contains(s);
}
