#include "command_handler.hpp"
#include "input_parser.hpp"
#include "lexical_analyzer.hpp"

#include <iostream>
#include <string>

#include "magic_enum.hpp"

int main()
{
	std::cout << "PySub Interpreter" << std::endl;
	std::cout << "Type \"help\" for commands or \"quit\" to exit." << std::endl;

	//LexicalAnalyzer lexical_analyzer;
	//ExpressionEvaluator expression_evaluator;

	CommandHandler command_handler;

	// read commands
	while (true) {
		std::cout << ">>> ";

		try {
			// get user input
			std::string input_line{};
			std::getline(std::cin, input_line);

			// perform lexical analysis on input
			TokenLine input_tokens = LexicalAnalyzer::GenerateTokens(input_line);
			auto command = CommandHandler::GetCommand(input_tokens);
			if (command) {
				auto argument = InputParser::GetCommandArgument(input_tokens);
				command_handler.Execute(command.value(), argument.value_or(""));
			}
			else {
			//	// evaluate as an expression
			//	int expression_value = expression_evaluator.evaluate(input_tokens);
			//	std::cout << expression_value << std::endl;
			}
		}
		catch (const std::exception& ex) {
			std::cout << ex.what() << std::endl;
		}
	}
}