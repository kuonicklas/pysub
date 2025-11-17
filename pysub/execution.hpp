#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include "globals.hpp"

#include <unordered_map>

// we avoid using inheritance by using std::variant and composition because I don't like heap allocation.

class Execution {
public:
	void RunCode(const std::vector<TokenLine>& token_lines);
	const std::unordered_map<std::string, ValueType>& GetSymbolTable() const;
private:
	std::unordered_map<std::string, ValueType> symbol_table{};
};

class FileExecution {
public:
	explicit FileExecution(const std::string& file_name);
	void Run();
	const std::vector<std::string>& GetFileLines() const;
	const std::vector<TokenLine>& GetFileTokens() const;
	const std::unordered_map<std::string, ValueType>& GetSymbolTable() const;
private:
	Execution execution{};
	std::vector<std::string> file_lines{};
	std::vector<TokenLine> file_tokens{};
};

class InterfaceExecution {
public:
	void Run(const std::vector<TokenLine>& token_lines);
	const std::unordered_map<std::string, ValueType>& GetSymbolTable() const;
private:
	Execution execution{};
};

#endif