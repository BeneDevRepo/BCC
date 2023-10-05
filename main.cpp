#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"


// constexpr const char *const code = R"(
// 	string s1 = "asdf";
// 	float s = (5-6-7) / 4.5;

// 	void a(float f, int g){
// 		int int0 = 1 + (2*3);

// 		if(int0) {
// 			int c = 7;
// 			int g = c + 6;
// 		}

// 		while(f) {
// 			int strg = 5 * -6;
// 			4 + 6 - 3;
// 		}
// 	}

// 	void b() {
// 	}
// )";

constexpr const char *const code = R"(
	float f(float x) { return x * x; }
	float b = f(0);
)";

void run() {
	Lexer lexer(code);

	std::cout << lexer << "\n";

	Parser parser(lexer);

	const Parser::Node* tree = parser.parse();

	std::cout << "Tree:  " << tree << "\n";
	tree->print();

	std::cout << "Reconstructed Source:\n" << tree->toString() << "\n";
}

int main() {
	try {
		run();
	} catch(const std::exception& e) {
		std::cout << "Exception thrown: " << e.what() << "\n";
	}

	return 0;
}

