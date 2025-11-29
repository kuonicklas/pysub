#include "execution.hpp"
#include "lexer.hpp"

#include <fstream>
#include <sstream>
#include <iostream>	// TEMP! REMOVE after RunCode is defined
#include <cassert>

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
	file_string = ReadEntireFile(filename);

	try {
		file_tokens = Lexer::GenerateTokens(file_string);
	}
	catch (const std::exception& ex) {
		throw Utilities::AddContext("lexer", ex);
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

std::string FileExecution::ReadEntireFile(std::filesystem::path file_path) {
	std::ifstream file_stream(file_path);
	if (file_stream.fail()) {
		throw std::runtime_error("error opening file");
	}
	const auto file_size = std::filesystem::file_size(file_path);
	std::string output(file_size, '\0');
	file_stream.read(output.data(), file_size);
	auto last = output.find_first_of('\0');	// file_size may exceed size of its contents, so we shrink
	output.resize(last);
	return output;
}

/* InterfaceExecution functions */

void InterfaceExecution::Run(const std::vector<Token>& tokens) {
	execution.RunCode(tokens);
}

const std::unordered_map<std::string, ValueType>& InterfaceExecution::GetSymbolTable() const {
	return execution.GetSymbolTable();
}