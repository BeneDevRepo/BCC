#pragma once


#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <vector>
#include <string>

#include "ScopedSymbolTable.hpp"
#include "ParseTree.hpp"
#include "AST.hpp"


class SemanticAnalyzer {
private:
public:
	inline SemanticAnalyzer() {}

public:
	inline static const AST::Node* visit(const ParseTree::Node* node, ScopedSymbolTable* scope) {
		switch(node->baseType()) {
			case ParseTree::Node::BaseType::EXPRESSION:
				return visit(dynamic_cast<const ParseTree::ExpressionNode*>(node), scope);
			case ParseTree::Node::BaseType::STATEMENT:
				return visit(dynamic_cast<const ParseTree::StatementNode*>(node), scope);
		}
	}

	inline static const AST::ExpressionNode* visit(const ParseTree::ExpressionNode* node, ScopedSymbolTable* scope) {
		switch(node->type()) {
		case ParseTree::Node::Type::CALL_EXPRESSION:
			return visit(dynamic_cast<const ParseTree::FunctionCallExpressionNode*>(node), scope);
		case ParseTree::Node::Type::GROUP_EXPRESSION:
			return nullptr;
		case ParseTree::Node::Type::UNARY_EXPRESSION:
			return nullptr;
		case ParseTree::Node::Type::BINARY_EXPRESSION:
			return nullptr;
		case ParseTree::Node::Type::VARIABLE_EXPRESSION:
			return nullptr;
		case ParseTree::Node::Type::LITERAL_EXPRESSION:
			return nullptr;
		}
	}

	inline static const AST::Node* visit(const ParseTree::StatementNode* node, ScopedSymbolTable* scope) {
		switch(node->type()) {
		case ParseTree::Node::Type::VARIABLE_DECLARATION:
			return nullptr;
		case ParseTree::Node::Type::EXPRESSION_STATEMENT:
			return nullptr;
		case ParseTree::Node::Type::BLOCK_STATEMENT:
			return nullptr;
		case ParseTree::Node::Type::RETURN_STATEMENT:
			return nullptr;
		case ParseTree::Node::Type::IF_STATEMENT:
			return nullptr;
		case ParseTree::Node::Type::WHILE_STATEMENT:
			return nullptr;
		case ParseTree::Node::Type::FUNCTION_DECLARATION:
			return nullptr;

		case ParseTree::Node::Type::PROGRAM:
			return nullptr;
		}
	}


	// Expressions:
	inline static const AST::ExpressionNode* visit(const ParseTree::FunctionCallExpressionNode* node, ScopedSymbolTable* scope) {
		std::vector<const AST::ExpressionNode*> astArgs;
		for(const ParseTree::ExpressionNode* arg : node->args)
			astArgs.push_back(arg->ast(scope));
		return new AST::FunctionCallExpressionNode(scope, node->name.value, astArgs);
	}

	inline static const AST::ExpressionNode* visit(const ParseTree::GroupExpressionNode* node, ScopedSymbolTable* scope) {
		return visit(node->a, scope);
	}


	// Statments:
};
