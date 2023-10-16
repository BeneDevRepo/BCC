#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "SymbolTable.hpp"
#include "Interpreter.hpp"


constexpr const char *const code = R"(
	float f(float x) {
		float x = 7;
		if(x)
			return (x + 1) * x;
	}

	float b = f(0);
)";

void run() {
	Lexer lexer(code);

	std::cout << lexer << "\n";

	Parser parser(lexer);

	const ParseTree::Program* tree = parser.program();

	std::cout << "ParseTree:  " << tree << "\n";
	tree->print("", true);

	std::cout << "\nReconstructed Source:\n" << tree->toString(0) << "\n\n";

	const AST::Node* ast = tree->ast();

	std::cout << "AST: " << ast << "\n";
	ast->print("", true);
	std::cout << "\n";

	SymbolTable symbols;
	ast->visit(symbols);
	symbols.print();

	Interpreter interpreter(ast, symbols);
	std::cout << "\nInterpreting:\n";
	interpreter.run();
}

int main() {
	try {
		run();
	} catch(const std::exception& e) {
		std::cout << "Exception thrown: " << e.what() << "\n";
	}

	return 0;
}

