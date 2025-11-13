#include "command_handler.hpp"
#include "input_parser.hpp"
#include "lexical_analyzer.hpp"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>

// returns true if any token contains a command
//bool CommandHandler::IsCommand(const TokenLine& token_line) {
//	return std::any_of(std::begin(token_line), std::end(token_line), [](const auto& token) -> bool {
//		return token.category == Category::Identifier && IsCommand(std::get<std::string>(token.value));
//		});
//}
//
//bool CommandHandler::IsCommand(const std::string& command) {
//	// string is used instead of string_view to avoid string_view -> string conversion
//	return GetCommand(command).has_value();
//}

std::optional<Command> CommandHandler::GetCommand(const TokenLine& token_line) noexcept {
	if (token_line.empty() || !std::holds_alternative<std::string>(token_line.front().value)) {
		return {};
	}
	return StringToCommand(std::get<std::string>(token_line.front().value));	// may or may not be valid
}

std::optional<Command> CommandHandler::StringToCommand(std::string_view command) {
	std::string lowercase = InputParser::ToLowerCase(command);
	if (lowercase == "quit") return Command::Quit;
	if (lowercase == "help") return Command::Help;
	if (lowercase == "read") return Command::Read;
	if (lowercase == "show") return Command::Show;
	if (lowercase == "clear") return Command::Clear;
	if (lowercase == "run") return Command::Run;
	return {};
}

std::string CommandHandler::CategoryToString(Category category) {
	switch (category) {
	case Category::Keyword:
		return "keyword";
	case Category::Identifier:
		return "identifier";
	case Category::StringLiteral:
		return "string_literal";
	case Category::NumericLiteral:
		return "numeric_literal";
	case Category::AssignmentOperator:
		return "assignment_operator";
	case Category::ArithmeticOperator:
		return "arithmetic_operator";
	case Category::LogicalOperator:
		return "logical_operator";
	case Category::RelationalOperator:
		return "relational_operator";
	case Category::LeftParenthesis:
		return "left_parenthesis";
	case Category::RightParenthesis:
		return "right_parenthesis";
	case Category::Colon:
		return "colon";
	case Category::Comma:
		return "comma";
	case Category::Comment:
		return "comment";
	case Category::Indent:
		return "indent";
	default:
		throw std::invalid_argument("unknown category");
	}
}

std::vector<std::string> CommandHandler::GetCommandList() {
	// hacky way to iterate through an enum and convert to strings
	// assumes that the enum values are auto-assigned!
	static std::vector<std::string> command_list{};
	static bool is_initialized = false;
	int command_num = 0;	
	while (!is_initialized) {
		switch (static_cast<Command>(command_num)) {
		case Command::Quit:
			command_list.push_back("quit");
			break;
		case Command::Help:
			command_list.push_back("help");
			break;
		case Command::Read:
			command_list.push_back("read");
			break;
		case Command::Show:
			command_list.push_back("show");
			break;
		case Command::Clear:
			command_list.push_back("clear");
			break;
		case Command::Run:
			command_list.push_back("run");
			is_initialized = true;	// keep this at the last enum
		// we don't set is_initialized in the default case because casting outside of enum bounds is UB.
		}
		++command_num;
	}
	return command_list;
}

void CommandHandler::Execute(const Command command, std::string_view argument) {
	// despite some overhead, we use a Command enum instead of a string for cleaner interface
	bool argument_provided = argument != "";
	switch (command) {
	case Command::Quit:
		if (argument_provided) {
			throw std::invalid_argument("quit called with unknown argument");
		}
		std::exit(0);
		break;
	case Command::Help:
		Help(argument);
		break;
	case Command::Read: {
		try {
			Read(std::string{ argument });	// copy is necessary here
			file_tokens = LexicalAnalyzer::GenerateTokens(file_lines);
		}
		catch (const std::exception& ex) {
			throw std::exception(std::string{std::string{"read error: "} + ex.what()}.c_str());
		}
		break;
	}
	case Command::Show:
		Show(argument);
		break;
	case Command::Clear:
		if (argument_provided) {
			throw std::invalid_argument("clear called with unknown argument");
		}
		ClearData();
		break;
	case Command::Run:
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
				for (const auto& command : GetCommandList()) {
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
	std::ifstream input_file(filename);
	if (input_file.fail()) {
		throw std::invalid_argument("error opening file");
	}
	if ((filename.size() < 3) || (filename.substr(filename.length() - 3, 3) != ".py"))
		throw std::invalid_argument("non-python files are not accepted");

	std::string line{};
	ClearData();
	while (std::getline(input_file, line)) {
		file_lines.push_back(line);
	}
	input_file.close();
}

void CommandHandler::Show(std::string_view argument) const {
	if (argument == "") {
		for (ptrdiff_t i = 0, size = file_lines.size(); i < size; ++i) {
			std::cout << '[' << i << ']' << file_lines.at(i) << std::endl;
		}
	}
	else if (argument == "tokens") {
		for (ptrdiff_t line_num = 0, num_lines = file_tokens.size(); line_num < num_lines; ++line_num) {
			std::cout << "Line " << line_num << ":" << std::endl;
			const auto& token_line = file_tokens.at(line_num);
			for (ptrdiff_t token_num = 0, num_tokens = token_line.size(); token_num < num_tokens; ++token_num) {
				const auto& curr_token = token_line.at(token_num);
				std::cout << '\t' << '[' << token_num << ']' << ": " << CategoryToString(curr_token.category) << "\t\t";
				std::visit([](const auto& v) {std::cout << v; }, curr_token.value);	// token contents
				std::cout << std::endl;
			}
		}
	}
	else if (argument == "variables") {
		// display symbol table contents
	}
	else {
		throw std::invalid_argument("invalid argument");
	}
}

void CommandHandler::ClearData() {
	file_lines.clear();
	file_tokens.clear();
}