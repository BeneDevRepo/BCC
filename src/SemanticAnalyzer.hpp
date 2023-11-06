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
			case ParseTree::Node::BaseType::PROGRAM:
				return visit(dynamic_cast<const ParseTree::Program*>(node), scope);
		}
	}

	inline static const AST::ExpressionNode* visit(const ParseTree::ExpressionNode* node, ScopedSymbolTable* scope) {
		switch(node->type()) {
		case ParseTree::ExpressionNode::Type::CALL_EXPRESSION:
			return visit(dynamic_cast<const ParseTree::FunctionCallExpressionNode*>(node), scope);
		case ParseTree::ExpressionNode::Type::GROUP_EXPRESSION:
			return visit(dynamic_cast<const ParseTree::GroupExpressionNode*>(node), scope);
		case ParseTree::ExpressionNode::Type::UNARY_EXPRESSION:
			return visit(dynamic_cast<const ParseTree::UnaryExpressionNode*>(node), scope);
		case ParseTree::ExpressionNode::Type::BINARY_EXPRESSION:
			return visit(dynamic_cast<const ParseTree::BinaryExpressionNode*>(node), scope);
		case ParseTree::ExpressionNode::Type::VARIABLE_EXPRESSION:
			return visit(dynamic_cast<const ParseTree::IdentifierNode*>(node), scope);
		case ParseTree::ExpressionNode::Type::LITERAL_EXPRESSION:
			return visit(dynamic_cast<const ParseTree::LiteralNode*>(node), scope);
		}

		// throw std::runtime_error("SemanticAnalyzer::visit(ExpressionNode): invalid expression Node type");
	}

	inline static const AST::StatementNode* visit(const ParseTree::StatementNode* node, ScopedSymbolTable* scope) {
		switch(node->type()) {
		case ParseTree::StatementNode::Type::VARIABLE_DECLARATION:
			return visit(dynamic_cast<const ParseTree::VariableDeclarationStatement*>(node), scope);
		case ParseTree::StatementNode::Type::EXPRESSION_STATEMENT:
			return visit(dynamic_cast<const ParseTree::ExpressionStatement*>(node), scope);
		case ParseTree::StatementNode::Type::BLOCK_STATEMENT:
			return visit(dynamic_cast<const ParseTree::BlockStatement*>(node), scope);
		case ParseTree::StatementNode::Type::RETURN_STATEMENT:
			return visit(dynamic_cast<const ParseTree::ReturnStatement*>(node), scope);
		case ParseTree::StatementNode::Type::IF_STATEMENT:
			return visit(dynamic_cast<const ParseTree::IfStatement*>(node), scope);
		case ParseTree::StatementNode::Type::WHILE_STATEMENT:
			return visit(dynamic_cast<const ParseTree::WhileStatement*>(node), scope);
		case ParseTree::StatementNode::Type::FUNCTION_DECLARATION:
			return visit(dynamic_cast<const ParseTree::FunctionDeclarationStatement*>(node), scope);
		}

		// throw std::runtime_error("SemanticAnalyzer::visit(StatementNode): invalid statement Node type");
	}


	// Expressions:
	inline static const AST::ExpressionNode* visit(const ParseTree::FunctionCallExpressionNode* node, ScopedSymbolTable* scope) {
		const std::string& name = node->name.value;
		if(!scope->lookupRecursive(name))
			throw std::runtime_error("Tried to call unknown function \"" + name + "\"");

		if(scope->lookupRecursive(name)->category != Symbol::Category::FUNCTION)
			throw std::runtime_error("Symbol \"" + name + "\" in Function call expression does not refer to a function.");
		
		std::vector<const AST::ExpressionNode*> astArgs;
		for(const ParseTree::ExpressionNode* arg : node->args)
			astArgs.push_back(visit(arg, scope));

		return new AST::FunctionCallExpressionNode(scope, name, astArgs);

	}

	inline static const AST::ExpressionNode* visit(const ParseTree::GroupExpressionNode* node, ScopedSymbolTable* scope) {
		return visit(node->a, scope);
	}

	inline static const AST::ExpressionNode* visit(const ParseTree::UnaryExpressionNode* node, ScopedSymbolTable* scope) {
		return new AST::UnaryExpressionNode(scope, node->op.value, visit(node->a, scope));
	}

	inline static const AST::ExpressionNode* visit(const ParseTree::BinaryExpressionNode* node, ScopedSymbolTable* scope) {
		return new AST::BinaryExpressionNode(scope, visit(node->a, scope), node->op.value, visit(node->b, scope));
	}

	inline static const AST::ExpressionNode* visit(const ParseTree::IdentifierNode* node, ScopedSymbolTable* scope) {
		const std::string& name = node->name.value;
		if(!scope->lookupRecursive(name))
			throw std::runtime_error("Use of undeclared identifier \"" + name + "\"");
		
		return new AST::IdentifierNode(scope, name);
	}

	inline static const AST::LiteralNode* visit(const ParseTree::LiteralNode* node, ScopedSymbolTable* scope) {
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wswitch" // suppress unhandled enumeration warning
		switch(node->value.type) {
			case Token::Type::BOOL_LITERAL:
				return new AST::BoolLiteralNode(scope, node->value.value == "true");
			case Token::Type::INT_LITERAL:
				return new AST::IntLiteralNode(scope, std::stoi(node->value.value)); // TODO: support all literal types
			case Token::Type::FLOAT_LITERAL:
				return new AST::FloatLiteralNode(scope, std::stof(node->value.value));
			case Token::Type::STRING_LITERAL:
				return new AST::StringLiteralNode(scope, node->value.value.substr(1, node->value.value.length()-2));
		}
		#pragma clang diagnostic pop
		throw std::runtime_error("Error generating literal AST Node: Token is not a known literal type");
	}


	// Statments:
	inline static const AST::StatementNode* visit(const ParseTree::VariableDeclarationStatement* node, ScopedSymbolTable* scope) {
		const std::string& typeName = node->typeName.value;
		const std::string& varName = node->varName.value;
		
		if(!scope->lookupRecursive(typeName))
				throw std::runtime_error("Unknown typename \"" + typeName + "\" in declaration of \"" + varName + "\"");
		if(scope->lookup(varName))
			throw std::runtime_error("Redeclaration of symbol \"" + varName + "\" in variable declaration");

		scope->declare(new Symbol(Symbol::Category::VARIABLE, varName, typeName));
		
		if(!node->expr) {
			return new AST::VariableDeclarationStatement(scope, typeName, varName);
		}

		const AST::ExpressionNode* expr = visit(node->expr, scope);
		const AST::VariableAssignmentStatement* assignment = new AST::VariableAssignmentStatement(scope, varName, expr);
		return new AST::VariableDeclarationStatement(scope, typeName, varName, assignment);
	}

	inline static const AST::StatementNode* visit(const ParseTree::ExpressionStatement* node, ScopedSymbolTable* scope) {
		return new AST::ExpressionStatement(scope, visit(node->expr, scope));
	}

	inline static const AST::StatementNode* visit(const ParseTree::BlockStatement* node, ScopedSymbolTable* scope) {
		ScopedSymbolTable* localScope = node->createScope ? new ScopedSymbolTable("Local Block Scope", scope) : scope;

		std::vector<const AST::StatementNode*> astStatements;

		for(const ParseTree::StatementNode* statement : node->statements)
			astStatements.push_back(visit(statement, localScope));

		return new AST::StatementList(localScope, astStatements);
	}

	inline static const AST::StatementNode* visit(const ParseTree::ReturnStatement* node, ScopedSymbolTable* scope) {
		return new AST::ReturnStatement(scope, visit(node->expr, scope));
	}

	inline static const AST::StatementNode* visit(const ParseTree::IfStatement* node, ScopedSymbolTable* scope) {
		return new AST::IfStatement(scope, visit(node->condition, scope), visit(node->body, scope));
	}

	inline static const AST::StatementNode* visit(const ParseTree::WhileStatement* node, ScopedSymbolTable* scope) {
		return new AST::WhileStatement(scope, visit(node->condition, scope), visit(node->body, scope));
	}

	inline static const AST::StatementNode* visit(const ParseTree::FunctionDeclarationStatement* node, ScopedSymbolTable* scope) {
		const std::string& typeName = node->typeName.value;
		const std::string& functionName = node->functionName.value;
		
		if(!scope->lookupRecursive(typeName))
			throw std::runtime_error("Error declaring function: Unknown return type \"" + typeName + "\"");
		if(scope->lookup(functionName))
			throw std::runtime_error("Error declaring function: Redeclaration of symbol \"" + functionName + "\"");
		
		ScopedSymbolTable* localScope = new ScopedSymbolTable("Local Function Scope", scope);

		std::vector<AST::FunctionDeclarationStatement::Argument> astArgs;
		for(const ParseTree::ArgumentsNode::Argument& arg : node->args->args)
			astArgs.push_back({ arg.type.value, arg.name.value });
	
		for(const AST::FunctionDeclarationStatement::Argument& arg : astArgs)
			localScope->declare(new Symbol(Symbol::Category::VARIABLE, arg.name, arg.type));

		AST::FunctionDeclarationStatement* decl = new AST::FunctionDeclarationStatement(localScope, node->typeName.value, node->functionName.value, astArgs, nullptr);
		scope->declare(new Symbol(Symbol::Category::FUNCTION, functionName, decl));

		const AST::StatementNode* body = visit(node->body, localScope);
		decl->body = body;

		// scope->overwrite(new Symbol(Symbol::Category::FUNCTION, functionName, decl));

		return decl;
	}

	inline static const AST::StatementList* visit(const ParseTree::Program* node, ScopedSymbolTable* scope) {
		std::vector<const AST::StatementNode*> astStatements;

		for(const ParseTree::StatementNode* statement : node->statements)
			astStatements.push_back(visit(statement, scope));
		
		return new AST::StatementList(scope, astStatements);
	}
};
