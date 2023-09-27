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
constexpr const char *const code = R"(
	1 - 8 + 7 + 6 * 5 * 3
)";

void run() {
	Lexer lexer;

	const auto tokens = lexer.tokenize(code);

	for(const auto& token : tokens) {
		std::cout << "" << token.value << " ";
		// std::cout << "Token " << (int)token.type << " Value: " << token.value << " Span: " << token.start << " - " << token.len << "\n";
	}
	std::cout << "\n";

	Parser parser;

	const Parser::Node* tree = parser.parse(tokens);

	std::cout << "Tree:\n";
	tree->print();

	(void)tree;
}

int main() {
	try {
		run();
	} catch(const std::exception& e) {
		std::cout << "Exception thrown: " << e.what() << "\n";
	}

	return 0;
}

