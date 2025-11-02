#ifndef COMMAND_HANDLER
#define COMMAND_HANDLER

#include "globals.hpp"
#include <set>
#include <optional>

class CommandHandler {
	public:
		enum class Command {
			Quit,
			Help,
			Read,
			Show,
			Clear
		};
		bool IsCommand(const TokenLine& token_line) const;
		bool IsCommand(const std::string& command) const;
		std::optional<Command> GetCommand(const TokenLine& token_line) const;
		std::optional<Command> GetCommand(std::string_view command) const;
		void Execute(const Command command) const;
};

#endif