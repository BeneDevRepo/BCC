#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"



constexpr const char *const code = R"(
	float f(float x) {
		if(x)
			return (x + 1) * x;
	}

	float b = f(0);
)";
// constexpr const char *const code = R"(
// 	(1 + 2) * 7 / F(x)
// )";

void run() {
	Lexer lexer(code);

	std::cout << lexer << "\n";

	Parser parser(lexer);

	const ParseTree::Program* tree = parser.parse();

	std::cout << "ParseTree:  " << tree << "\n";
	tree->print("", true);

	std::cout << "\nReconstructed Source:\n" << tree->toString(0) << "\n\n";

	const AST::Node* ast = tree->ast();

	std::cout << "AST: " << ast << "\n";
	ast->print("", true);
}

int main() {
	try {
		run();
	} catch(const std::exception& e) {
		std::cout << "Exception thrown: " << e.what() << "\n";
	}

	return 0;
}

