#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"


// constexpr const char *const code = R"(
// int main() {
// 	int var_a = 0;
// 	float var_b = a + 5.;
// 	string str = "asdf";
// 	return var_b;
// }
// )";
// constexpr const char *const code = R"(
// 	{
// 		int a = 1 - 8 + 7 + 6 * 5 * 3;
// 		int b = 1 + 2 * 3 - (a / 2.);
// 		string d;
// 		string e = "asdf";
// 		int c = -(-(-(128)));
// 	}
// )";
constexpr const char *const code = R"(
	string s = "asdf";
	float s = (5-6-7) / 4.5;

	void a(float f, int g){
		int int0 = 1 + (2*3);

		if(int0) {
			int c = 7;
			int g = c + 6;
		}

		while(f) {
			int strg = 5 * -6;
			4 + 6 - 3;
		}
	}

	void b() {
	}
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

