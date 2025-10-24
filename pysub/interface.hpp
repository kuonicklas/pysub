#ifndef INTERFACE_HPP
#define INTERFACE_HPP

class Interface{
public:
	void start();
	int add(int l, int r) {
		return l + r;
	}

private:
	/*std::vector<std::string> programCode;

	void help(std::string);
	void read(std::string);
	void show(std::string);
	void clear();

	bool isCommand(const tokenLineType);
	std::string getCommand(const tokenLineType);
	std::string getArgument(const tokenLineType);
	void execute(std::string, std::string);
	void evaluate(const tokenLineType);
	void removeWhitespaces(std::string& input_line);*/
};

#endif