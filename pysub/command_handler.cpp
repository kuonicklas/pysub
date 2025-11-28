#include "command_handler.hpp"
#include "input_parser.hpp"
#include "lexical_analyzer.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>

// returns true if any token contains a command
//bool CommandHandler::IsCommand(const std::vector<Token>& token_line) {
//	return std::any_of(std::begin(token_line), std::end(token_line), [](const auto& token) -> bool {
//		return token.category == Category::Identifier && IsCommand(std::get<std::string>(token.value));
//		});
//}
//
//bool CommandHandler::IsCommand(const std::string& command) {
//	// string is used instead of string_view to avoid string_view -> string conversion
//	return GetCommand(command).has_value();
//}

std::optional<Command> CommandHandler::GetCommand(const std::vector<Token>& token_line) noexcept {
	if (token_line.empty() || !std::holds_alternative<std::string>(token_line.front().value)) {
		return {};
	}
	return StringToCommand(std::get<std::string>(token_line.front().value));	// may or may not be valid
}

std::optional<Command> CommandHandler::StringToCommand(std::string_view command) {
	return magic_enum::enum_cast<Command>(command, magic_enum::case_insensitive);
}

std::string CommandHandler::CategoryToString(Category category) {
	auto category_string = magic_enum::enum_name(category);
	if (category_string.empty()) {
		throw std::invalid_argument("unknown category");
	}
	return std::string{ category_string };
}

void CommandHandler::Execute(const Command command, std::string_view argument) {
	// despite some overhead, we use a Command enum instead of a string for cleaner interface
	bool argument_provided = argument != "";
	switch (command) {
	case Command::Quit:
		if (argument_provided) {
			throw std::invalid_argument("quit() called with unknown argument");
		}
		std::exit(0);
		break;
	case Command::Help:
		Help(argument);
		break;
	case Command::Read: {
		try {
			Read(std::string{ argument });	// copy is necessary here
		}
		catch (const std::exception& ex) {
			throw InputParser::AddContext("read error", ex);
		}
		break;
	}
	case Command::Show:
		Show(argument);
		break;
	case Command::Clear:
		if (argument_provided) {
			throw std::invalid_argument("clear() called with unknown argument");
		}
		ClearData();
		break;
	case Command::Run:
		if (argument_provided) {
			throw std::invalid_argument("run() called with unknown argument");
		}
		Run();
		break;
	default:
		throw std::invalid_argument("invalid command enum");
	}
}

void CommandHandler::Help(std::string_view initial_argument) {
	bool initial_argument_provided = initial_argument != "";
	if (!initial_argument_provided) {
		// display interface
		std::cout << "Help Utility" << std::endl;
		std::cout << "\t* For info on a specific command, enter its name." << std::endl;
		std::cout << "\t* For a list of commands, enter \"commands\"." << std::endl;
		std::cout << "\t* To exit the utility, enter \"exit\"." << std::endl;
	}

	do {
		std::string input_line{};

		if (!initial_argument_provided) {
			// get argument from user
			std::cout << "help> ";
			std::getline(std::cin, input_line);
			InputParser::TrimLeadingAndTrailingWhitespaces(input_line);
			InputParser::ToLowerCase(input_line);
		}
		std::string_view argument = initial_argument_provided ? initial_argument : input_line;

		std::optional<Command> input_command = StringToCommand(argument);
		if (!input_command.has_value()) {
			if (argument == "commands") {
				constexpr auto command_list = GetCommandList();
				for (const auto& command : command_list) {
					std::cout << '\t' << command;
				}
				std::cout << std::endl;
			}
			else if (!initial_argument_provided && argument == "exit") {
				return;
			}
			else if (argument != "") {
				// if no argument was provided, simply request again.
				// if argument was provided but unrecognizeable, print error and request again.
				std::cout << "Command not recognized." << std::endl;
			}
		}
		else {
			switch (input_command.value()) {
			case Command::Quit:
				std::cout << "Closes the window." << std::endl;
				break;
			case Command::Help:
				std::cout << "Opens the help utility. Use help(commandname) to directly display info about a command." << std::endl;
				break;
			case Command::Read:
				std::cout << "Reads a .py file: read(filename.py). Can be a filepath." << std::endl;
				break;
			case Command::Show:
				std::cout << "Displays the contents of the imported .py file." << std::endl;
				std::cout << "\tshow(tokens)\tDisplays the tokens generated from lexical analysis." << std::endl;
				std::cout << "\tshow(variables)\t Displays the symbol table containing stored variables." << std::endl;
				break;
			case Command::Clear:
				std::cout << "Clears any file data from any prior \"read\" command." << std::endl;
				break;
			case Command::Run:
				std::cout << "Executes the file imported using the \"read\" command." << std::endl;
				break;
			}
		}
	} while (!initial_argument_provided);
}

// const std::string& instead of std::string_view because file opening does not support std::string_view ._.
void CommandHandler::Read(const std::string& filename) {
	FileExecution new_file_execution(filename);
	curr_execution = new_file_execution;
}

void CommandHandler::Show(std::string_view argument) const {
	if (argument == "") {
		if (!std::holds_alternative<FileExecution>(curr_execution)) {
			throw std::invalid_argument("No file has been opened!");
		}
		const auto& file_string = std::get<FileExecution>(curr_execution).GetFileString();
		std::cout << file_string << std::endl;
	}
	else if (argument == "tokens") {
		if (!std::holds_alternative<FileExecution>(curr_execution)) {
			throw std::invalid_argument("No file has been opened!");
		}
		const auto& file_tokens = std::get<FileExecution>(curr_execution).GetFileTokens();
		PrintTokenLine(file_tokens);
	}
	else if (argument == "variables") {
		// display symbol table contents
		const std::unordered_map<std::string, ValueType>* symbol_table{};
		std::visit([&symbol_table](const auto& v) {symbol_table = &v.GetSymbolTable(); }, curr_execution);
		PrintSymbolTable(symbol_table);
	}
	else {
		throw std::invalid_argument("invalid argument");
	}
}

void CommandHandler::Run() {
	if (!std::holds_alternative<FileExecution>(curr_execution)) {
		throw std::invalid_argument("No file to run!");
	}
	std::get<FileExecution>(curr_execution).Run();
}

void CommandHandler::ClearData() {
	curr_execution = InterfaceExecution{};
}

void CommandHandler::PrintTokenLine(const std::vector<Token>& token_line) {
	constexpr size_t max_category_length = GetMaxCategoryLength();

	for (ptrdiff_t i = 0, num_tokens = token_line.size(), line_num = 0; i < num_tokens; ++i) {
		const auto& curr_token = token_line.at(i);
		if (i == 0 || (i > 0 && token_line.at(i - 1).category == Category::Newline)) {
			// wait until after a newline token is processed to start next line, except for the first
			std::cout << "Line " << line_num++ << ":" << std::endl;
		}
		std::cout << '\t' << '[' << i << ']' << ": " << std::left << std::setw(max_category_length + 1) << CategoryToString(curr_token.category);
		std::visit([](const auto& v) {std::cout << v; }, curr_token.value);
		std::cout << std::endl;
	}
}

void CommandHandler::PrintSymbolTable(const std::unordered_map<std::string, ValueType>* symbol_table) {
	for (const auto& pair : *symbol_table) {
		std::cout << pair.first << " = ";
		std::visit([](const auto& v) {std::cout << v; }, pair.second);
		std::cout << std::endl;
	}
}