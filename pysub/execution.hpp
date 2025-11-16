#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include "globals.hpp"

#include <unordered_map>

class Execution {
public:
	void RunCode(const std::vector<TokenLine>& token_lines);
	const auto& GetSymbolTable() const;
private:
	std::unordered_map<std::string, ValueType> symbol_table{};
};

class FileExecution {
public:
	explicit FileExecution(const std::string& file_name);
	void Run();
private:
	Execution execution{};
	std::vector<std::string> file_lines{};
	std::vector<TokenLine> file_tokens{};
};

class InterfaceExecution {
public:
	void Run(const std::vector<TokenLine>& token_lines);
private:
	Execution execution{};
};

#endif