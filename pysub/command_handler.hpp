#ifndef COMMAND_HANDLER
#define COMMAND_HANDLER

#include "globals.hpp"
#include <set>

class CommandHandler {
	public:
		bool IsCommand(const TokenLine& token_line) const;
		bool IsCommand(const std::string& s) const;	
	private:
		inline static const std::set<std::string> command_list{
			"quit",
			"help",
			"read",
			"show",
			"clear"
		};
};

#endif