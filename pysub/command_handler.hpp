#ifndef COMMAND_HANDLER
#define COMMAND_HANDLER

#include "globals.hpp"

#include <optional>


// note: enum values must be auto-assigned! (we iterate over them occasionally)
enum class Command {
	Quit,
	Help,
	Read,
	Show,
	Clear,
	Run
};

class CommandHandler {
public:
	/*static bool IsCommand(const TokenLine& token_line);
	static bool IsCommand(const std::string& command);*/
	static std::optional<Command> GetCommand(const TokenLine& token_line) noexcept;
	static std::optional<Command> StringToCommand(std::string_view command);
	static std::string CategoryToString(Category category);

	static std::vector<std::string> GetCommandList();
	void Execute(const Command command, std::string_view argument = "");
	static void Help(std::string_view argument = "");
	void Read(const std::string& filename);
	void Show(std::string_view argument) const;
	void ClearData();
private:
	std::vector<std::string> file_lines{};
	std::vector<TokenLine> file_tokens{};
};

#endif