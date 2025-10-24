#include "command_handler.hpp"
#include "globals.hpp"

#include <iostream>
#include <string>

int main()
{
	std::cout << "PySub Interpreter" << std::endl;
	std::cout << "Type \"help\" for commands or \"quit\" to exit." << std::endl;

	InputParser input_parser;
	CommandHandler command_handler;
	LexicalAnalyzer lexical_analyzer;
	ExpressionEvaluator expression_evaluator;

	// read commands
	while (true) {
		std::cout << ">>> ";

		try {
			// get user input
			std::string input_line{};
			std::getline(std::cin, input_line);

			// perform lexical analysis on input
			TokenLine input_tokens = lexical_analyzer.generate(input_line);

			if (command_handler.IsCommand(input_tokens)) {
				// execute as a command
				std::string command = input_parser.GetCommand(input_tokens);
				std::string argument = input_parser.GetArgument(input_tokens);
				command_handler.execute(command, argument);
			}
			else
			{
				// evaluate as an expression
				int expression_value = expression_evaluator.evaluate(input_tokens);
				std::cout << expression_value << std::endl;
			}
		}
		catch (const std::exception& ex) {
			std::cout << ex.what() << std::endl;
		}
	}
}