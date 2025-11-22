#ifndef EXECUTION_HPP
#define EXECUTION_HPP

#include "globals.hpp"

#include <unordered_map>

// we avoid using inheritance by using std::variant and composition because I don't like heap allocation.

class Execution {
public:
	void RunCode(const std::vector<Token>& tokens);
	const std::unordered_map<std::string, ValueType>& GetSymbolTable() const;
private:
	std::unordered_map<std::string, ValueType> symbol_table{};
};

class FileExecution {
public:
	explicit FileExecution(const std::string& file_name);
	void Run();
	const std::string& GetFileString() const;
	const std::vector<Token>& GetFileTokens() const;
	const std::unordered_map<std::string, ValueType>& GetSymbolTable() const;
private:
	Execution execution{};
	std::string file_string{};
	std::vector<Token> file_tokens{};
};

class InterfaceExecution {
public:
	void Run(const std::vector<Token>& tokens);
	const std::unordered_map<std::string, ValueType>& GetSymbolTable() const;
private:
	Execution execution{};
};

#endif