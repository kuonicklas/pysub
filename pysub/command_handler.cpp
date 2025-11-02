#include "command_handler.hpp"
#include "input_parser.hpp"

#include <algorithm>

// returns true if any token contains a command
bool CommandHandler::IsCommand(const TokenLine& token_line) const {
	return std::any_of(std::begin(token_line), std::end(token_line), [this](const auto& token) -> bool {
		return token.category == Category::Identifier && this->IsCommand(token.string);
		});
}

bool CommandHandler::IsCommand(const std::string& command) const {
	// string is used instead of string_view to avoid string_view -> string conversion
	return GetCommand(command).has_value();
}

std::optional<CommandHandler::Command> CommandHandler::GetCommand(const TokenLine& token_line) const {
	if (token_line.empty()) {
		return {};
	}
	return GetCommand(token_line.front().string);	// may or may not be valid
}

std::optional<CommandHandler::Command> CommandHandler::GetCommand(std::string_view command) const {
	std::string lowercase = InputParser::ToLowerCase(command);
	if (lowercase == "quit") return Command::Quit;
	if (lowercase == "help") return Command::Help;
	if (lowercase == "read") return Command::Read;
	if (lowercase == "show") return Command::Show;
	if (lowercase == "clear") return Command::Clear;
	return {};
}

void CommandHandler::Execute(Command command) const {
	// despite some overhead, we use a Command enum instead of a string for cleaner interface
	
}