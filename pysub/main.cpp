#include "interface.hpp"
#include <exception>
#include <iostream>

int main()
{
	try {
		Interface interface;
		interface.start();
	}
	catch (const std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}
}