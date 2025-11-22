#include "execution.hpp"
#include "lexical_analyzer.hpp"
#include "input_parser.hpp"

#include <fstream>
#include <sstream>

#include <iostream>	// TEMP! REMOVE after RunCode is defined

/* Execution functions */

void Execution::RunCode(const std::vector<Token>& tokens) {
	// do stuff with interpreter
	size_t i = tokens.size();
	std::cout << i << std::endl;	// temp line of code to avoid warnings
}

const std::unordered_map<std::string, ValueType>& Execution::GetSymbolTable() const {
	return symbol_table;
}

/* FileExecution functions */

FileExecution::FileExecution(const std::string& filename) {
	// read file
	std::ifstream input_file(filename);
	if (input_file.fail()) {
		throw std::invalid_argument("error opening file");
	}
	if ((filename.size() < 3) || (filename.substr(filename.length() - 3, 3) != ".py")) {
		throw std::invalid_argument("non-python files are not accepted");
	}

	std::ostringstream file_contents{};
	file_contents << input_file.rdbuf();
	file_string = file_contents.str();	// replace with the read method that avoids string duplication in sstream

	auto  input_file.rdbuf();

	while (std::getline(input_file, line, '\0')) {
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

const std::string& FileExecution::GetFileString() const {
	return file_string;
}
const std::vector<Token>& FileExecution::GetFileTokens() const {
	return file_tokens;
}

const std::unordered_map<std::string, ValueType>& FileExecution::GetSymbolTable() const {
	return execution.GetSymbolTable();
}

/* InterfaceExecution functions */

void InterfaceExecution::Run(const std::vector<Token>& tokens) {
	execution.RunCode(tokens);
}

const std::unordered_map<std::string, ValueType>& InterfaceExecution::GetSymbolTable() const {
	return execution.GetSymbolTable();
}