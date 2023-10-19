#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "ScopedSymbolTable.hpp"
#include "Interpreter.hpp"


constexpr const char *const code = R"(
	float f(int x) {
		if(x==1) return 1;
		if(x==2) return 1;
		return f(x - 1) + f(x - 2);
	}

	float b = f(5);
)";

void run() {
	Lexer lexer(code);

	std::cout << lexer << "\n";

	Parser parser(lexer);

	const ParseTree::Program* tree = parser.program();

	std::cout << "ParseTree:  " << tree << "\n";
	tree->print("", true);

	std::cout << "\nReconstructed Source:\n" << tree->toString(0) << "\n\n";

	ScopedSymbolTable globalScope("Global Scope");
	globalScope.declare(new Symbol(Symbol::Category::TYPE, "void", "__VOID__"));
	globalScope.declare(new Symbol(Symbol::Category::TYPE, "bool", "__BOOL__"));
	globalScope.declare(new Symbol(Symbol::Category::TYPE, "int", "__INT__"));
	globalScope.declare(new Symbol(Symbol::Category::TYPE, "float", "__FLOAT__"));
	globalScope.declare(new Symbol(Symbol::Category::TYPE, "string", "__STRING__"));

	const AST::Node* ast = tree->ast(&globalScope);

	std::cout << "AST: " << ast << "\n";
	ast->print("", true);
	std::cout << "\n";

	ast->visit();
	globalScope.print();

	Interpreter interpreter(ast);
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

