#pragma once


#include <stdexcept>
#include <iostream>
#include <variant>
#include <cstdint>
#include <vector>
#include <string>

#include "AST.hpp"
#include "ScopedSymbolTable.hpp"


struct Value {
public:
	enum class Special : uint8_t {
		VOID, RETURN, BREAK, CONTINUE,
	};

private:
	std::variant<Special, bool, int, float, std::string> value;

public:
	inline Value(const Special s): value(s) {}
	inline Value(const bool b): value(b) {}
	inline Value(const int b): value(b) {}
	inline Value(const float b): value(b) {}
	inline Value(const std::string& b): value(b) {}
	inline static Value Void() { return Value(Special::VOID); }
	inline static Value Return() { return Value(Special::RETURN); }
	inline static Value Break() { return Value(Special::BREAK); }
	inline static Value Continue() { return Value(Special::CONTINUE); }

public:
	inline bool isVoid() const { return is<void>(); }
	inline bool isConvertibleToBool() const { return is<bool>() || is<int>(); }
	inline bool isControl() const {
		if(!is<Special>())
			return false;
		const Special s = get<Special>();
		return s==Special::RETURN || s==Special::BREAK || s==Special::CONTINUE;
	}
	inline bool toBool() const {
		if(is<bool>()) return get<bool>();
		if(is<int>()) return get<int>();
		throw std::runtime_error("Tried to convert non-bool-converible Value to bool");
	}


	template<typename T>
	inline bool is() const { return std::holds_alternative<T>(value); }

	template<>
	inline bool is<void>() const { return is<Special>() && get<Special>()==Special::VOID; }

	template<typename T>
	inline T get() const { return std::get<T>(value); }


	inline std::string toString() const {
		if(isVoid()) return "<VOID>";
		if(is<bool>()) return (get<bool>() ? "true" : "false");
		if(is<int>()) return "<int>" + std::to_string(get<int>());
		if(is<float>()) return "<float>" + std::to_string(get<float>());
		if(is<std::string>()) return "<string>\"" + get<std::string>() + "\"";
		throw std::runtime_error("Error printing interpreter value: unknown variant type");
	}
};


struct Variable {
	enum class Category : uint8_t {
		BOOL, INT, FLOAT, STRING,
	} type;
	std::string name;
	Value value;
	// std::string value;
	// inline Variable(const Category type, const std::string& name, const std::string& value): type(type), name(name), value(value) {}
	inline Variable(const Category type, const std::string& name, const Value& value): type(type), name(name), value(value) {}
};

class ScopedVariableTable {
private:
	std::string scopeName;
	std::unordered_map<std::string, Variable*> symbols;

public:
	ScopedVariableTable* parent;

public:
	inline ScopedVariableTable(const std::string& scopeName, ScopedVariableTable* parent = nullptr): scopeName(scopeName), parent(parent) {
	}

	// inline void set(const std::string& name, const std::string& value) {
	inline void set(const std::string& name, const Value& value) {
		if(symbols.contains(name)) {
			symbols[name]->value = value;
		}

		symbols[name] = new Variable(Variable::Category::INT, name, value); // TODO: correct type
	}

	inline const Variable* lookup(const std::string& name) const {
		for(const ScopedVariableTable* table = this; table != nullptr; table = table->parent)
			if(table->symbols.contains(name))
				return table->symbols.at(name);
		throw std::runtime_error("ScopedVariableTable::lookup(): Tried to lookup unknown symbol \"" + name + "\"");
	}

	inline void print(const std::string& indent) const {
		std::cout << indent << "<Variable Table \"" + scopeName + "\">:\n";
		for(const auto& [name, symbol] : symbols) {
			std::cout << indent << name << ": ";
			switch(symbol->type) {
				case Variable::Category::BOOL:
					std::cout << "<Bool>";
					break;
				case Variable::Category::INT:
					std::cout << "<Int>";
					break;
				case Variable::Category::FLOAT:
					std::cout << "<Float>";
					break;
				case Variable::Category::STRING:
					std::cout << "<String>";
					break;
			}
			std::cout << " " << symbol->value.toString() << "\n";
		}
		std::cout << indent << "</Variable Table \"" + scopeName + "\">\n\n";
	}
};


class Interpreter {
private:
	const AST::Node* ast;
	ScopedVariableTable globalVariables;
	std::string indent;

public:
	inline Interpreter(const AST::Node* ast): ast(ast), globalVariables("Global Scope") {
	}

	inline void run() {
		switch(ast->baseType()) {
		case AST::Node::BaseType::EXPRESSION:
			visit(&globalVariables, dynamic_cast<const AST::ExpressionNode*>(ast));
			break;

		case AST::Node::BaseType::STATEMENT:
			visit(&globalVariables, dynamic_cast<const AST::StatementNode*>(ast));
			break;
		}

		globalVariables.print(indent);
	}

	inline Value visit(ScopedVariableTable* scope, const AST::ExpressionNode* node) {
		switch(node->type()) {
			case AST::Node::Type::LITERAL_EXPRESSION:
				return visitLiteralExpression(scope, dynamic_cast<const AST::LiteralNode*>(node));
			case AST::Node::Type::VARIABLE_EXPRESSION:
				return visitVariableExpression(scope, dynamic_cast<const AST::IdentifierNode*>(node));
			case AST::Node::Type::UNARY_EXPRESSION:
				return visitUnaryExpression(scope, dynamic_cast<const AST::UnaryExpressionNode*>(node));
			case AST::Node::Type::BINARY_EXPRESSION:
				return visitBinaryExpression(scope, dynamic_cast<const AST::BinaryExpressionNode*>(node));
			case AST::Node::Type::CALL_EXPRESSION:
				return visitFunctionCall(scope, dynamic_cast<const AST::FunctionCallExpressionNode*>(node));
		}
	}

	inline Value visit(ScopedVariableTable* scope, const AST::StatementNode* node) {
		switch(node->type()) {
			case AST::Node::Type::EXPRESSION_STATEMENT:
				return visit(scope, dynamic_cast<const AST::ExpressionStatement*>(node)->expr);
			case AST::Node::Type::STATEMENT_LIST:
				return visitStatementList(scope, dynamic_cast<const AST::StatementList*>(node));
			case AST::Node::Type::RETURN_STATEMENT:
				return visitReturnStatement(scope, dynamic_cast<const AST::ReturnStatement*>(node));

			case AST::Node::Type::IF_STATEMENT:
				return visitIfStatement(scope, dynamic_cast<const AST::IfStatement*>(node));
			case AST::Node::Type::WHILE_STATEMENT:
				return visitWhileStatement(scope, dynamic_cast<const AST::WhileStatement*>(node));
			case AST::Node::Type::FUNCTION_DECLARATION_STATEMENT:
				return visitFunctionDeclaration(scope, dynamic_cast<const AST::FunctionDeclarationStatement*>(node));
			case AST::Node::Type::VARIABLE_DECLARATION_STATEMENT:
				return visitVariableDeclaration(scope, dynamic_cast<const AST::VariableDeclarationStatement*>(node));
			case AST::Node::Type::VARIABLE_ASSIGNMENT_STATEMENT:
				return visitVariableAssignment(scope, dynamic_cast<const AST::VariableAssignmentStatement*>(node));
		}
		throw std::runtime_error("Interpreter::visit(): unknown Node type");
	}

	Value visitLiteralExpression(ScopedVariableTable* scope, const AST::LiteralNode* node) {
		Value out = Value::Void();

		switch(node->type) {
			case AST::LiteralNode::Type::BOOL:
				// ret = dynamic_cast<const AST::BoolLiteralNode*>(node)->value ? "true" : "false";
				out = Value(dynamic_cast<const AST::BoolLiteralNode*>(node)->value);
				break;
			case AST::LiteralNode::Type::INT:
				// ret = std::to_string(dynamic_cast<const AST::IntLiteralNode*>(node)->value);
				out = Value(dynamic_cast<const AST::IntLiteralNode*>(node)->value);
				break;
			case AST::LiteralNode::Type::FLOAT:
				// ret = std::to_string(dynamic_cast<const AST::FloatLiteralNode*>(node)->value);
				out = Value(dynamic_cast<const AST::FloatLiteralNode*>(node)->value);
				break;
			case AST::LiteralNode::Type::STRING:
				// ret = dynamic_cast<const AST::StringLiteralNode*>(node)->value;
				out = Value(dynamic_cast<const AST::StringLiteralNode*>(node)->value);
				break;
		}

		if(out.isVoid())
			throw std::runtime_error("Interpreter::visitLiteralExpression: Unknown Literal Type");

		// std::cout << indent << "<LiteralExpression " + ret + "/> => " << ret << "\n";
		std::cout << indent << "<LiteralExpression " << out.toString() << "/> => " << out.toString() << "\n";

		return out;
	}

	Value visitVariableExpression(ScopedVariableTable* scope, const AST::IdentifierNode* node) {
		const Value ret = scope->lookup(node->name)->value;

		std::cout << indent << "<VariableExpression \"" + node->name + "\"/> => " << ret.toString() << "\n";

		return ret;
	}

	Value visitUnaryExpression(ScopedVariableTable* scope, const AST::UnaryExpressionNode* node) {
		std::cout << indent << "<UnaryExpression " + node->opString + ">:\n";
		
		Value res = Value::Void();
		
		const Value a = visit(scope, node->a);

		switch(node->op) {
			case AST::UnaryExpressionNode::Operation::PLUS:
				if(a.is<int>())  res = a.get<int>(); break;
				if(a.is<float>())  res = a.get<float>(); break;
			case AST::UnaryExpressionNode::Operation::MINUS:
				if(a.is<int>())  res = -a.get<int>(); break;
				if(a.is<float>())  res = -a.get<float>(); break;
		}

		if(res.isVoid())
			throw std::runtime_error("Interpreter::visitUnaryExpression: Invalid type or operator in Unary Expression");
		
		std::cout << indent << "</UnaryExpression> => " << res.toString() << "\n";

		return res;
	}

	Value visitBinaryExpression(ScopedVariableTable* scope, const AST::BinaryExpressionNode* node) {
		std::cout << indent << "<BinaryExpression " + node->opString() + ">:\n";

		Value res = Value::Void();

		indent += "  ";

		const Value a = visit(scope, node->a);

		const Value b = visit(scope, node->b);

		switch(node->op) {
			case AST::BinaryExpressionNode::Operation::PLUS:
				if(a.is<int>() && b.is<int>())     res = a.get<int>()   + b.get<int>(); break;
				if(a.is<int>() && b.is<float>())   res = a.get<int>()   + b.get<float>(); break;
				if(a.is<float>() && b.is<int>())   res = a.get<float>() + b.get<int>(); break;
				if(a.is<float>() && b.is<float>()) res = a.get<float>() + b.get<float>(); break;
			case AST::BinaryExpressionNode::Operation::MINUS:
				if(a.is<int>() && b.is<int>())     res = a.get<int>()   - b.get<int>(); break;
				if(a.is<int>() && b.is<float>())   res = a.get<int>()   - b.get<float>(); break;
				if(a.is<float>() && b.is<int>())   res = a.get<float>() - b.get<int>(); break;
				if(a.is<float>() && b.is<float>()) res = a.get<float>() - b.get<float>(); break;
			case AST::BinaryExpressionNode::Operation::MUL:
				if(a.is<int>() && b.is<int>())     res = a.get<int>()   * b.get<int>(); break;
				if(a.is<int>() && b.is<float>())   res = a.get<int>()   * b.get<float>(); break;
				if(a.is<float>() && b.is<int>())   res = a.get<float>() * b.get<int>(); break;
				if(a.is<float>() && b.is<float>()) res = a.get<float>() * b.get<float>(); break;
			case AST::BinaryExpressionNode::Operation::DIV:
				if(a.is<int>() && b.is<int>())     res = a.get<int>()   / b.get<int>(); break;
				if(a.is<int>() && b.is<float>())   res = a.get<int>()   / b.get<float>(); break;
				if(a.is<float>() && b.is<int>())   res = a.get<float>() / b.get<int>(); break;
				if(a.is<float>() && b.is<float>()) res = a.get<float>() / b.get<float>(); break;

			case AST::BinaryExpressionNode::Operation::COMP_EQ:
				if(a.is<int>() && b.is<int>())     res = a.get<int>()   == b.get<int>(); break;
				if(a.is<int>() && b.is<float>())   res = a.get<int>()   == b.get<float>(); break;
				if(a.is<float>() && b.is<int>())   res = a.get<float>() == b.get<int>(); break;
				if(a.is<float>() && b.is<float>()) res = a.get<float>() == b.get<float>(); break;
			case AST::BinaryExpressionNode::Operation::COMP_NE:
				if(a.is<int>() && b.is<int>())     res = a.get<int>()   != b.get<int>(); break;
				if(a.is<int>() && b.is<float>())   res = a.get<int>()   != b.get<float>(); break;
				if(a.is<float>() && b.is<int>())   res = a.get<float>() != b.get<int>(); break;
				if(a.is<float>() && b.is<float>()) res = a.get<float>() != b.get<float>(); break;
			case AST::BinaryExpressionNode::Operation::COMP_GT:
				if(a.is<int>() && b.is<int>())     res = a.get<int>()   > b.get<int>(); break;
				if(a.is<int>() && b.is<float>())   res = a.get<int>()   > b.get<float>(); break;
				if(a.is<float>() && b.is<int>())   res = a.get<float>() > b.get<int>(); break;
				if(a.is<float>() && b.is<float>()) res = a.get<float>() > b.get<float>(); break;
			case AST::BinaryExpressionNode::Operation::COMP_LT:
				if(a.is<int>() && b.is<int>())     res = a.get<int>()   < b.get<int>(); break;
				if(a.is<int>() && b.is<float>())   res = a.get<int>()   < b.get<float>(); break;
				if(a.is<float>() && b.is<int>())   res = a.get<float>() < b.get<int>(); break;
				if(a.is<float>() && b.is<float>()) res = a.get<float>() < b.get<float>(); break;
			case AST::BinaryExpressionNode::Operation::COMP_GE:
				if(a.is<int>() && b.is<int>())     res = a.get<int>()   >= b.get<int>(); break;
				if(a.is<int>() && b.is<float>())   res = a.get<int>()   >= b.get<float>(); break;
				if(a.is<float>() && b.is<int>())   res = a.get<float>() >= b.get<int>(); break;
				if(a.is<float>() && b.is<float>()) res = a.get<float>() >= b.get<float>(); break;
			case AST::BinaryExpressionNode::Operation::COMP_LE:
				if(a.is<int>() && b.is<int>())     res = a.get<int>()   <= b.get<int>(); break;
				if(a.is<int>() && b.is<float>())   res = a.get<int>()   <= b.get<float>(); break;
				if(a.is<float>() && b.is<int>())   res = a.get<float>() <= b.get<int>(); break;
				if(a.is<float>() && b.is<float>()) res = a.get<float>() <= b.get<float>(); break;
		}

		if(res.isVoid())
			throw std::runtime_error("Interpreter::visitUnaryExpression: Invalid type or operator in Binary Expression");

		indent = indent.substr(0, indent.size() - 2);

		std::cout << indent << "</BinaryExpression> => " << res.toString() << "\n";

		return res;
	}

	Value visitFunctionCall(ScopedVariableTable* scope, const AST::FunctionCallExpressionNode* node) {
		ScopedVariableTable* localScope = new ScopedVariableTable("Local FunctionCall Scope", scope);
		std::cout << indent << "<FunctionCall \"" + node->name + "\">:\n";

		indent += "  ";

		const AST::FunctionDeclarationStatement* targetFunction =
			dynamic_cast<const AST::FunctionDeclarationStatement*>(
				std::get<const AST::Node*>(
					node->getScope().lookupRecursive(node->name)->type
				)
			);

		for(size_t i = 0; i < node->args.size(); i++) {
			const AST::FunctionDeclarationStatement::Argument& param = targetFunction->args[i];
			// const std::string& paramType = param.type;
			const std::string& paramName = param.name;
			const Value val = visit(scope, node->args[i]);
			localScope->set(paramName, val);
		}

		const Value out = visit(localScope, targetFunction->body);

		indent = indent.substr(0, indent.size() - 2);

		std::cout << indent << "</FunctionCall> => " << out.toString() << "\n";

		localScope->print(indent);

		return out;
	}


	// Stetements:
	Value visitStatementList(ScopedVariableTable* scope, const AST::StatementList* node) {
		std::cout << indent << "<StatementList>\n";
	
		indent += "  ";

		Value out = Value::Void();

		for(const AST::StatementNode* statement : node->statements) {
			out = visit(scope, statement);

			if(!out.isVoid()) break; // found return, break or continue statement // TODO: fix
		}
		
		indent = indent.substr(0, indent.size() - 2);

		std::cout << indent << "</StatementList>\n";

		return out;
	}

	Value visitReturnStatement(ScopedVariableTable* scope, const AST::ReturnStatement* node) {
		std::cout << indent << "<ReturnStatement>\n";

		indent += "  ";

		const Value out = visit(scope, node->expr);

		indent = indent.substr(0, indent.size() - 2);

		return out;
	}



	Value visitIfStatement(ScopedVariableTable* scope, const AST::IfStatement* node) {
		std::cout << indent << "<IfStatement>\n";

		indent += "  ";

		const Value cond = visit(scope, node->condition);

		if(!cond.isConvertibleToBool())
			throw std::runtime_error("Interpreter::visitIfStatement: condition not convertible to bool");
		
		Value res = Value::Void();

		if(cond.toBool()) {
			ScopedVariableTable* localScope = new ScopedVariableTable("Local IfStatement Scope", scope);

			res = visit(localScope, node->body);
		}

		indent = indent.substr(0, indent.size() - 2);

		std::cout << indent << "<IfStatement/>\n";

		return res; // TODO: fix
	}

	Value visitWhileStatement(ScopedVariableTable* scope, const AST::WhileStatement* node) {
		std::cout << indent << "<WhileStatement>\n";

		indent += "  ";

		// visit(scope, node->expr); // TODO: implement

		indent = indent.substr(0, indent.size() - 2);

		std::cout << indent << "<WhileStatement/>\n";

		return Value::Void(); // TODO: fix
	}

	Value visitFunctionDeclaration(ScopedVariableTable* scope, const AST::FunctionDeclarationStatement* node) {
		std::cout << indent << "<FunctionDeclaration> (skipping)\n";

		return Value::Void();
	}

	Value visitVariableDeclaration(ScopedVariableTable* scope, const AST::VariableDeclarationStatement* node) {
		std::cout << indent << "<VariableDeclaration>\n";

		indent += "  ";

		if(node->initialAssignment)
			visit(scope, node->initialAssignment);

		indent = indent.substr(0, indent.size() - 2);

		return Value::Void();
	}

	Value visitVariableAssignment(ScopedVariableTable* scope, const AST::VariableAssignmentStatement* node) {
		std::cout << indent << "<VariableAssignment \"" + node->varName + "\">\n";

		indent += "  ";

		Value val = visit(scope, node->expr);
		scope->set(node->varName, val);

		indent = indent.substr(0, indent.size() - 2);

		return Value::Void();
	}
};

