#include "execution.hpp"
#include "lexical_analyzer.hpp"
#include "input_parser.hpp"

#include <fstream>

void Execution::RunCode(const std::vector<TokenLine>& token_lines) {
	// do stuff with interpreter
}

const auto& Execution::GetSymbolTable() const {
	return symbol_table;
}

explicit FileExecution::FileExecution(const std::string& filename) {
	// read file
	std::ifstream input_file(filename);
	if (input_file.fail()) {
		throw std::invalid_argument("error opening file");
	}
	if ((filename.size() < 3) || (filename.substr(filename.length() - 3, 3) != ".py")) {
		throw std::invalid_argument("non-python files are not accepted");
	}

	std::string line{};
	while (std::getline(input_file, line)) {
		file_lines.push_back(line);
	}
	input_file.close();

	// get tokens
	try {
		file_tokens = LexicalAnalyzer::GenerateTokens(file_lines);
	}
	catch (const std::exception& ex) {
		throw InputParser::AddContext("input parser: ", ex);
	}
}

void FileExecution::Run() {
	Execution new_execution;
	new_execution.RunCode(file_tokens);
	// we want to reset the context every time the file is run (as opposed to InterfaceExecution, whose context is persistent).
	execution = new_execution;
}

void InterfaceExecution::Run(const std::vector<TokenLine>& token_lines) {
	execution.RunCode(token_lines);
}