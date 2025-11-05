#include "command_handler.hpp"
#include "input_parser.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>

// returns true if any token contains a command
bool CommandHandler::IsCommand(const TokenLine& token_line) {
	return std::any_of(std::begin(token_line), std::end(token_line), [](const auto& token) -> bool {
		return token.category == Category::Identifier && IsCommand(token.string);
		});
}

bool CommandHandler::IsCommand(const std::string& command) {
	// string is used instead of string_view to avoid string_view -> string conversion
	return GetCommand(command).has_value();
}

std::optional<Command> CommandHandler::GetCommand(const TokenLine& token_line) {
	if (token_line.empty()) {
		return {};
	}
	return GetCommand(token_line.front().string);	// may or may not be valid
}

std::optional<Command> CommandHandler::GetCommand(std::string_view command) {
	std::string lowercase = InputParser::ToLowerCase(command);
	if (lowercase == "quit") return Command::Quit;
	if (lowercase == "help") return Command::Help;
	if (lowercase == "read") return Command::Read;
	if (lowercase == "show") return Command::Show;
	if (lowercase == "clear") return Command::Clear;
	return {};
}

void CommandHandler::Execute(const Command command, std::string_view argument) {
	// despite some overhead, we use a Command enum instead of a string for cleaner interface
	switch (command) {
	case Command::Quit:
		if (argument != "") {
			throw std::exception("quit called with unknown argument");
		}
		std::exit(0);
		break;
	case Command::Help:
		Help(argument);
		break;
	case Command::Read:
		break;
	case Command::Show:
		break;
	case Command::Clear:
		break;
	default:
		throw std::exception("unknown command");
	}
	
}

void CommandHandler::Help(std::string_view initial_argument) const {
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

		// execute
		if (argument == "quit") {
			std::cout << "Closes the window." << std::endl;
		}
		else if (argument == "help") {
			std::cout << "Opens the help utility. Use help(commandname) to directly display info about a command." << std::endl;
		}
		else if (argument == "read") {
			std::cout << "Reads a .py file. Use the following form: read(filename.py). The file must be located in the same directory as the interpreter." << std::endl;
		}
		else if (argument == "show") {
			std::cout << "Displays the contents of the most recently imported .py file." << std::endl;
		}
		else if (argument == "clear") {
			std::cout << "Clears any file data from any prior \"read\" command." << std::endl;
		}
		else if (argument == "commands") {
			std::cout << '\t' << "Quit" << '\t' << "Help" << '\t' << "Read" << std::endl;
			std::cout << '\t' << "Show" << '\t' << "Clear" << std::endl;
		}
		else if (!initial_argument_provided && argument == "exit") {
			return;
		}
		if (argument != "") {
			// if no argument was provided, simply request again.
			// if argument was provided but unrecognizeable, print error and request again.
			std::cout << "Command not recognized." << std::endl;
		}
	} while (true);
}