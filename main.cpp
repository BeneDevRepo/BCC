#include <iostream>
#include <streambuf>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"
#include "ScopedSymbolTable.hpp"
#include "Interpreter.hpp"


constexpr const char *const code = R"(
	float f(int x) {
		if(x==1) return 1;
		if(x==2) return 1;
		return f(x - 1) + f(x - 2);
	}

	int b = f(5);
	string a = "asdf " + true + b + " ; " + 1 + (2 + 3);
)";

class DummyLogger: private std::streambuf, public std::ostream {
public:
	inline DummyLogger(): std::ostream(this) {}
};

void run() {
	DummyLogger dout;
	// std::ostream& cout = dout;
	std::ostream& cout = std::cout;

	Lexer lexer(code);
	Parser parser(lexer);

	cout << lexer << "\n";

	const ParseTree::Program* tree = parser.program();

	tree->print(cout, "", true);

	cout << "\nReconstructed Source:\n" << tree->toString(0) << "\n\n";

	ScopedSymbolTable globalScope("Global Scope");
	globalScope.declare(new Symbol(Symbol::Category::TYPE, "void", "__VOID__"));
	globalScope.declare(new Symbol(Symbol::Category::TYPE, "bool", "__BOOL__"));
	globalScope.declare(new Symbol(Symbol::Category::TYPE, "int", "__INT__"));
	globalScope.declare(new Symbol(Symbol::Category::TYPE, "float", "__FLOAT__"));
	globalScope.declare(new Symbol(Symbol::Category::TYPE, "string", "__STRING__"));

	const AST::Node* ast = SemanticAnalyzer::visit(tree, &globalScope);

	cout << "AST: " << ast << "\n";
	ast->print(cout, "", true);
	cout << "\n";

	globalScope.print(cout);

	Interpreter interpreter(ast, cout);
	cout << "\nInterpreting:\n";
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

