#pragma once


#include <iostream>

#include "AST.hpp"
#include "SymbolTable.hpp"


class Interpreter {
private:
	const AST::Node* ast;
	SymbolTable symbols;

public:
	inline Interpreter(const AST::Node* ast, const SymbolTable& symbols): ast(ast), symbols(symbols) {
	}

	inline void run() {
		visit(ast);
	}

	inline void visit(const AST::Node* node) {
		switch(node->type()) {
			case AST::Node::Type::LITERAL_EXPRESSION:
			case AST::Node::Type::VARIABLE_EXPRESSION:
			case AST::Node::Type::UNARY_EXPRESSION:
			case AST::Node::Type::BINARY_EXPRESSION:
			case AST::Node::Type::CALL_EXPRESSION:
				return;

			case AST::Node::Type::EXPRESSION_STATEMENT:
				return;
			case AST::Node::Type::STATEMENT_LIST:
				return visitStatementList(dynamic_cast<const AST::StatementList*>(node));
			case AST::Node::Type::RETURN_STATEMENT:
				return;

			case AST::Node::Type::IF_STATEMENT:
			case AST::Node::Type::WHILE_STATEMENT:
			case AST::Node::Type::FUNCTION_DECLARATION_STATEMENT:
				return visitFunctionDeclaration(dynamic_cast<const AST::FunctionDeclarationStatement*>(node));
			case AST::Node::Type::VARIABLE_DECLARATION_STATEMENT:
				return visitVariableDeclaration(dynamic_cast<const AST::VariableDeclarationStatement*>(node));
			case AST::Node::Type::VARIABLE_ASSIGNMENT_STATEMENT:
				return;
		}
		throw std::runtime_error("Interpreter::visit(): unknown Node type");
	}

	void visitStatementList(const AST::StatementList* node) {
		std::cout << "Visiting StatementList\n";

		for(const AST::StatementNode* statement :  node->statements)
			visit(statement);
	}

	void visitFunctionDeclaration(const AST::FunctionDeclarationStatement* node) {
		std::cout << "Visiting FunctionDeclaration (skipping)\n";
	}

	void visitVariableDeclaration(const AST::VariableDeclarationStatement* node) {
		std::cout << "Visiting VariableDeclaration\n";
	}
};

