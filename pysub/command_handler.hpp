#ifndef COMMAND_HANDLER
#define COMMAND_HANDLER

#include "globals.hpp"

#include <optional>


enum class Command {
	Quit,
	Help,
	Read,
	Show,
	Clear
};

class CommandHandler {
public:
	static bool IsCommand(const TokenLine& token_line);
	static bool IsCommand(const std::string& command);
	static std::optional<Command> GetCommand(const TokenLine& token_line);
	static std::optional<Command> GetCommand(std::string_view command);
	void Execute(const Command command, std::string_view argument);
	void Help(std::string_view argument) const;
private:
	std::vector<std::string> file_lines{};
	std::vector<TokenLine> file_tokens{};
};

#endif