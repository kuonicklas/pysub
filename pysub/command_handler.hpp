#ifndef COMMAND_HANDLER
#define COMMAND_HANDLER

#include "globals.hpp"
#include "execution.hpp"

#include "magic_enum/magic_enum.hpp"

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

	static constexpr auto GetCommandList();
	static constexpr size_t GetMaxCategoryLength();
	void Execute(const Command command, std::string_view argument = "");
	static void Help(std::string_view argument = "");
	void Read(const std::string& filename);
	void Show(std::string_view argument) const;
	void Run();
	void ClearData();
private:
	std::variant<InterfaceExecution, FileExecution> curr_execution{};	// InterfaceExecution is the default
};

constexpr auto CommandHandler::GetCommandList() {
	return magic_enum::enum_names<Command>();
}

constexpr size_t CommandHandler::GetMaxCategoryLength() {
	return std::max_element(std::begin(magic_enum::enum_names<Category>()), std::end(magic_enum::enum_names<Category>()), [](const auto& l, const auto& r) {return l.size() < r.size(); })->size();
}

#endif