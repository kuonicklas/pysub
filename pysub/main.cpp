#include "command_handler.hpp"
#include "input_parser.hpp"
#include "lexical_analyzer.hpp"

#include <iostream>
#include <string>

int main()
{
	std::cout << "PySub Interpreter" << std::endl;
	std::cout << "Type \"help\" for commands or \"quit\" to exit." << std::endl;

	CommandHandler command_handler;

	while (true) {
		std::cout << ">>> ";

		try {
			// get input
			std::string input_line{};
			std::getline(std::cin, input_line);

			// process input
			std::vector<Token> input_tokens = LexicalAnalyzer::GenerateTokens(input_line);
			auto command = CommandHandler::GetCommand(input_tokens);

			// evaluate
			if (command) {
				// ...as a command
				auto argument = InputParser::GetCommandArgument(input_tokens);
				command_handler.Execute(command.value(), argument.value_or(""));
			}
			else {
				// ...as code
				// 
				// if is conditional
				//		while input
				//			grab more lines
				//	run
				// 
				//	int expression_value = expression_evaluator.evaluate(input_tokens);
				//	std::cout << expression_value << std::endl;
			}
		}
		catch (const std::exception& ex) {
			std::cout << ex.what() << std::endl;
		}
	}
}