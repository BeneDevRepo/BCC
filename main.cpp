#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"



constexpr const char *const code = R"(
	float f(float x) {
		return x * x;
	}

	float b = f(0);
)";

void run() {
	Lexer lexer(code);

	std::cout << lexer << "\n";

	Parser parser(lexer);

	const Parser::Node* tree = parser.parse();

	std::cout << "Tree:  " << tree << "\n";
	tree->print();

	std::cout << "\nReconstructed Source:\n" << tree->toString() << "\n";
}

int main() {
	try {
		run();
	} catch(const std::exception& e) {
		std::cout << "Exception thrown: " << e.what() << "\n";
	}

	return 0;
}

