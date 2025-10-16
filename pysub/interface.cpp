#include "interface.hpp"
#include <iostream>
//#include <string>
//#include <fstream>
//#include <iomanip>

void Interface::start() {
	//Welcome Message
	std::cout << "PySub Interpreter" << std::endl;
	std::cout << "Type \"help\" for commands or \"quit\" to exit." << std::endl;

	//Read-Eval Loop
	while (true) {
		std:: cout << ">>> ";

		try {
			//Get Line of User Input
			string input_line;
			getline(cin, input_line);

			//Lexically Analyze the Command, get tokens
			auto tokenLine = generate(input_line);

			//Identify and Execute
			if (isCommand(tokenLine)) {
				//Execute as Command
				string command = getCommand(tokenLine);
				string argument = getArgument(tokenLine);
				execute(command, argument);
			}
			else
			{
				//Evaluate as Expression
				evaluate(tokenLine);
			}
		}
		catch (const std::string& errorMsg) {
			std::cout << errorMsg << std::endl;
		}
	}
}
//
//bool Interface::isCommand(const tokenLineType tokenLine) {
//
//	for (auto pair : tokenLine) {
//		//Check for command keywords
//		if (pair.second == categoryType::IDENTIFIER) {
//			if (pair.first == "quit" || pair.first == "help" || pair.first == "read" ||
//				pair.first == "show" || pair.first == "clear")
//				return true;
//		}
//		//Check for operands and operators
//		if (pair.second == categoryType::NUMERIC_LITERAL ||
//			pair.second == categoryType::ARITH_OP ||
//			pair.second == categoryType::LOGICAL_OP ||
//			pair.second == categoryType::RELATIONAL_OP) {
//			return false;
//		}
//	}
//
//	return true; //If no keywords nor operators/operands, assume a malformed command
//}
//
//string Interface::getCommand(const tokenLineType tokenLine) {
//	auto iter = tokenLine.begin();
//
//	if (iter == tokenLine.end()) return ""; //No command
//
//	return iter->first; //Fetches first token as a command
//}
//
//string Interface::getArgument(const tokenLineType tokenLine) {
//	auto iter = tokenLine.begin();
//	string argument;
//
//	//First token is command, we ignore
//	if (iter == tokenLine.end()) return ""; //No command, so no argument either
//
//	iter++;
//
//	try {
//		//Second token is either nothing or open parenthesis
//		if (iter == tokenLine.end()) return "";
//
//		if (iter->second != categoryType::LEFT_PAREN) throw 1;
//
//		iter++;
//
//		//Third token is argument or closed parenthesis
//		if (iter == tokenLine.end()) throw 2;
//		if (iter->second == categoryType::RIGHT_PAREN) return "";
//
//		//Grab everything until closed parenthesis
//		while (iter != tokenLine.end() && iter->second != categoryType::RIGHT_PAREN) {
//			argument += iter->first;
//			iter++;
//		}
//
//		//nth token is closed parentheses
//		if (iter == tokenLine.end()) throw 2;
//		if (iter->second != categoryType::RIGHT_PAREN) throw 2;
//
//		iter++;
//
//		//n + 1th token is end
//		if (iter != tokenLine.end()) throw 3;
//	}
//	catch (int errorNum) {
//		//Throw correct error message
//		switch (errorNum) {
//		case 1:
//			throw string("Open parenthesis missing or in wrong place!");
//		case 2:
//			throw string("Missing closed parenthesis!");
//		case 3:
//			throw string("Unknown content after parenthesis!");
//		default:
//			throw string("Unknown error when reading argument!");
//		}
//	}
//
//	return argument;
//}
//
//void Interface::execute(string command, string argument) {
//	try {
//		//Execute appropriate command
//		if (command == "quit") {
//			if (argument != "") throw 1;
//
//			exit(0);
//		}
//
//		if (command == "help") {
//			help(argument);
//			return;
//		}
//
//		if (command == "read") {
//			read(argument); //Read into programCode
//			auto tokens = generate(programCode); //Get tokens
//			save(tokens); //Store tokens
//			return;
//		}
//
//		if (command == "show") {
//			show(argument);
//			return;
//		}
//
//		if (command == "clear") {
//			if (argument != "") throw 1;
//
//			clear();
//			return;
//		}
//
//		if (command == "") return;
//
//		throw 2; //Command does not match available commands
//	}
//	catch (int errorNum) {
//		switch (errorNum) {
//		case 1:
//			throw string("Command accepts no arguments!");
//		case 2:
//			throw string("Unknown command!");
//		default:
//			throw string("Unknown execution error!");
//		}
//	}
//}
//
//void Interface::evaluate(const tokenLineType tokenLine) {
//	try {
//		int result = ExpEvaluator::evaluate(tokenLine); //Get value
//		cout << result << endl; //Output value
//	}
//	catch (string errorMsg) {
//		throw errorMsg;
//	}
//}
//
//void Interface::help(string argument) {
//	//Display welcome message
//	if (argument == "") {
//		cout << "Welcome the help utility! Available commands:" << endl;
//		cout << "\t* Enter a command for a description." << endl;
//		cout << "\t* Type \"commands\" for a list of interface commands." << endl;
//		cout << "\t* Type \"exit\" to return to the interpreter." << endl;
//	}
//
//	//Read-Eval-Print Loop
//	do {
//		string input_line;
//
//		//If no argument given, get from user
//		if (argument == "") {
//			cout << "help> ";
//
//			//Get User Input
//			getline(cin, input_line);
//			removeWhitespaces(input_line);
//		}
//		else
//		{
//			input_line = argument;
//		}
//
//		//Execute
//		if (input_line == "quit") {
//			cout << "Closes the interpreter window." << endl;
//		}
//		else if (input_line == "help") {
//			cout << "Opens the help utility. help(commandname) can directly display info about a command without entering the utility." << endl;
//		}
//		else if (input_line == "read") {
//			cout << "Import a Python file for use. Use the following form: read(filename.py). The file must be located in the same directory as the interpreter." << endl;
//		}
//		else if (input_line == "show") {
//			cout << "Display, line by line, the contents of an imported .py file." << endl;
//		}
//		else if (input_line == "clear") {
//			cout << "Clear the interpreter of any files imported using the \"read\" command." << endl;
//		}
//		else if (input_line == "commands") {
//			cout << left;
//			cout << "\t" << setw(8) << "Quit" << setw(8) << "Help" << setw(8) << "Read" << endl;
//			cout << "\t" << setw(8) << "Show" << setw(8) << "Clear" << endl;
//		}
//		else if (input_line == "exit") {
//			if (argument != "") //The exit command only exists within the help utility, not the interpreter itself.
//				cout << "The command does not exist!" << endl;
//			return;
//		}
//		else if (input_line != "") {
//			cout << "The command does not exist!" << endl;
//		}
//	} while (argument == "");
//}
//
//void Interface::removeWhitespaces(string& input_line) {
//	string result;
//	bool startReached, endReached;
//	int endIndex = -1;
//
//	startReached = endReached = false;
//
//	//Traverse from the end to the first non-whitespace character
//	for (int i = input_line.length() - 1; i >= 0; i--) {
//		char c = input_line[i];
//
//		if (c != ' ') {
//			endIndex = i;
//			break;
//		}
//	}
//
//	//Traverse String and Modify
//	for (int i = 0; i <= endIndex; i++) {
//		char c = input_line[i];
//
//		//Exclude all whitespace chars from beginning
//		if (c != ' ')
//			startReached = true;
//
//		if (startReached) {
//			result += c; //Add
//		}
//	}
//
//	input_line = result;
//}
//
//void Interface::read(string filename) {
//	//Check if it is .py file
//	if ((filename.length() < 3) || (filename.substr(filename.length() - 3, 3) != ".py")) {
//		throw string("Not a .py (Python) file!");
//	}
//
//	ifstream inputFile(filename);
//
//	if (inputFile.fail()) {
//		throw string("File could not be opened or does not exist!");
//	}
//
//	//Read from file into vector
//	string line;
//
//	clear(); //Clear prior saved content
//
//	while (getline(inputFile, line)) {
//		programCode.push_back(line);
//	}
//
//	inputFile.close();
//}
//
//void Interface::show(string argument) {
//	if (argument == "") {
//		int lineNumber = 0;
//
//		//Display the file line by line
//		for (auto line : programCode) {
//			cout << "[" << lineNumber << "]" << line << endl;
//			lineNumber++;
//		}
//	}
//	else if (argument == "tokens") {
//		LexicalAnalyzer::show();
//	}
//	else
//		throw string("Unknown argument!");
//
//}
//
//void Interface::clear() {
//	programCode.clear();
//	LexicalAnalyzer::clear();
//}