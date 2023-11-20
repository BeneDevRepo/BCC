#pragma once


#include <stdexcept>
#include <iostream>
#include <variant>
#include <cstdint>
#include <vector>
#include <string>

#include "AST.hpp"
#include "ScopedSymbolTable.hpp"


inline std::string operator+(const std::string& a, const int v) { return a + std::to_string(v); }
inline std::string operator+(const std::string& a, const float v) { return a + std::to_string(v); }
inline std::string operator+(const std::string& a, const bool b) { return a + (b ? "true" : "false"); }


// string [- * /] [bool float int string]  =>  string  <exception>
#define STRING_OP_DEFINE_TYPE(OP, T) \
	inline std::string OP(const std::string& a, const T& b) { throw std::runtime_error("Tried to execute placeholder string-operation std::string " #OP " " #T); }
#define STRING_OP_DEFINE(OP) \
	STRING_OP_DEFINE_TYPE(OP, bool) \
	STRING_OP_DEFINE_TYPE(OP, float) \
	STRING_OP_DEFINE_TYPE(OP, int) \
	STRING_OP_DEFINE_TYPE(OP, std::string)
STRING_OP_DEFINE(operator-)
STRING_OP_DEFINE(operator*)
STRING_OP_DEFINE(operator/)
#undef STRING_OP_DEFINE_TYPE
#undef STRING_OP_DEFINE


// string [== != > < >= <=] [bool float int string]  =>  bool  <exception>
#define STRING_OP_DEFINE_TYPE(OP, T) \
	inline bool OP(const std::string& a, const T& b) { throw std::runtime_error("Tried to execute placeholder string-operation std::string " #OP " " #T); }
#define STRING_OP_DEFINE(OP) \
	STRING_OP_DEFINE_TYPE(OP, bool) \
	STRING_OP_DEFINE_TYPE(OP, float) \
	STRING_OP_DEFINE_TYPE(OP, int)
STRING_OP_DEFINE(operator==)
STRING_OP_DEFINE(operator!=)
STRING_OP_DEFINE(operator>)
STRING_OP_DEFINE(operator<)
STRING_OP_DEFINE(operator>=)
STRING_OP_DEFINE(operator<=)
#undef STRING_OP_DEFINE_TYPE
#undef STRING_OP_DEFINE


// T [+ - * /] string  =>  string  <string [+ - * /] T>
#define STRING_OP_CORRECT(OP_NAME, OP) \
	template<typename T> \
		requires ( \
			requires (T a, const std::string& b) { \
				{b OP a} -> std::convertible_to<std::string>; \
			} \
			&& !std::convertible_to<std::remove_cvref_t<T>, std::string> \
		)  \
	inline std::string OP_NAME(const T& a, const std::string& b) { return b OP a; }
STRING_OP_CORRECT(operator+, +)
STRING_OP_CORRECT(operator-, -)
STRING_OP_CORRECT(operator*, *)
STRING_OP_CORRECT(operator/, /)
#undef STRING_OP_CORRECT

// T [== != > < >= <=] string  =>  bool  <string [== != > < >= <=] T>  ||  <exception>
#define STRING_OP_CORRECT(OP_NAME, OP) \
	template<typename T> \
		requires ( \
			!std::convertible_to<std::remove_cvref_t<T>, std::string> && \
			requires (T a, const std::string& b) {{b OP a} -> std::convertible_to<bool>; } \
		)  \
	inline bool OP_NAME(const T& a, const std::string& b) { return b OP a; } \
	template<typename T> \
		requires ( \
			!std::convertible_to<std::remove_cvref_t<T>, std::string> && \
			!requires (T a, const std::string& b) {{b OP a} -> std::convertible_to<bool>; } \
		)  \
	inline bool OP_NAME(const T& a, const std::string& b) { throw std::runtime_error("Tried to execute placeholder string-operation std::string " #OP " " "UnknownType!!1!"); }
STRING_OP_CORRECT(operator==, ==)
STRING_OP_CORRECT(operator!=, !=)
STRING_OP_CORRECT(operator>, >)
STRING_OP_CORRECT(operator<, <)
STRING_OP_CORRECT(operator>=, >=)
STRING_OP_CORRECT(operator<=, <=)
#undef STRING_OP_CORRECT


struct StatementResult {
public:
	enum class Type : uint8_t {
		VOID, RETURN, BREAK, CONTINUE,
	};

private:
	Type type_;

public:
	inline StatementResult(const Type type): type_(type) {}
	inline static StatementResult Void() { return StatementResult(Type::VOID); }
	inline static StatementResult Return() { return StatementResult(Type::RETURN); }
	inline static StatementResult Break() { return StatementResult(Type::BREAK); }
	inline static StatementResult Continue() { return StatementResult(Type::CONTINUE); }

public:
	inline Type type() const { return type_; }
};

struct Value {
private:
	std::variant<std::monostate, bool, int, float, std::string> value; // std::monostate -> no value

public:
	inline Value(void): value{} {}
	inline Value(const bool b): value(b) {}
	inline Value(const int b): value(b) {}
	inline Value(const float b): value(b) {}
	inline Value(const std::string& b): value(b) {}

public:
	template<typename T>
	inline bool is() const { return std::holds_alternative<T>(value); }
	inline bool isVoid() const { return is<std::monostate>(); }
	inline bool isConvertibleToBool() const { return is<bool>() || is<int>(); }

	template<typename T>
	inline T get() const { return std::get<T>(value); }

	template<typename T>
	inline T to() const;

	template<>
	inline bool to<bool>() const {
		if(is<bool>()) return get<bool>();
		if(is<int>()) return get<int>();
		throw std::runtime_error("Value::convert: Tried to convert non-bool-converible Value to bool");
	}

	template<>
	inline int to<int>() const {
		if(is<bool>()) return get<bool>();
		if(is<int>()) return get<int>();
		throw std::runtime_error("Value::convert: Tried to convert non-int-converible Value to int");
	}

	template<>
	inline float to<float>() const {
		if(is<bool>()) return get<bool>();
		if(is<int>()) return get<int>();
		if(is<float>()) return get<float>();
		throw std::runtime_error("Value::convert: Tried to convert non-float-converible Value to float");
	}

	template<>
	inline std::string to<std::string>() const {
		if(is<bool>()) return get<bool>() ? "true" : "false";
		if(is<int>()) return std::to_string(get<int>());
		if(is<float>()) return std::to_string(get<float>());
		if(is<std::string>()) return get<std::string>();
		throw std::runtime_error("Value::convert: Tried to convert non-string-converible Value to string");
	}

	inline std::string toString() const {
		if(isVoid()) return "<VOID>";
		if(is<bool>()) return (get<bool>() ? "true" : "false");
		if(is<int>()) return "<int>" + std::to_string(get<int>());
		if(is<float>()) return "<float>" + std::to_string(get<float>());
		if(is<std::string>()) return "<string>\"" + get<std::string>() + "\"";
		throw std::runtime_error("Error printing interpreter value: unknown variant type");
	}

public:
	#define typeCaseAB(TA, OP, TB) \
		if(is<TA>() && other.is<TB>()) return get<TA>() OP other.get<TB>();
	#define typeCaseA(TA, OP) \
		typeCaseAB(TA, OP, bool) \
		typeCaseAB(TA, OP, int) \
		typeCaseAB(TA, OP, float) \
		typeCaseAB(TA, OP, std::string)
	#define opImpl(OP_NAME, OP) \
		inline Value OP_NAME(const Value& other) const { \
			typeCaseA(bool, OP) \
			typeCaseA(int, OP) \
			typeCaseA(float, OP) \
			typeCaseA(std::string, OP) \
			throw std::runtime_error("Error executing Interpreter::Value::" #OP_NAME "(): unsipported types " + toString() + ", " + other.toString()); \
		}
	opImpl(operator+, +)
	opImpl(operator-, -)
	opImpl(operator*, *)
	opImpl(operator/, /)
	opImpl(operator==, ==)
	opImpl(operator!=, !=)
	opImpl(operator>, >)
	opImpl(operator<, <)
	opImpl(operator>=, >=)
	opImpl(operator<=, <=)
	#undef opImpl
	#undef typeCaseA
	#undef typeCaseAB
};


struct Variable {
	enum class Category : uint8_t {
		BOOL, INT, FLOAT, STRING,
	} type;
	std::string name;
	Value value;
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

	inline void set(const std::string& name, const Value& value) {
		if(symbols.contains(name)) {
			symbols[name]->value = value;
		}

		Variable::Category category = static_cast<Variable::Category>(-1);
		if(value.is<bool>())
			category = Variable::Category::BOOL;
		if(value.is<int>())
			category = Variable::Category::INT;
		if(value.is<float>())
			category = Variable::Category::FLOAT;
		if(value.is<std::string>())
			category = Variable::Category::STRING;
		symbols[name] = new Variable(category, name, value); // TODO: correct type
	}

	inline const Variable* lookup(const std::string& name) const {
		for(const ScopedVariableTable* table = this; table != nullptr; table = table->parent)
			if(table->symbols.contains(name))
				return table->symbols.at(name);
		throw std::runtime_error("ScopedVariableTable::lookup(): Tried to lookup unknown symbol \"" + name + "\"");
	}

	inline void print(const std::string& indent) const {
		std::cout << indent << "<Variable Table \"" + scopeName + "\">:\n";
		for(const auto& [name, value] : symbols) {
			std::cout << indent << name << ": " << value->value.toString() << "\n";
		}
		std::cout << indent << "</Variable Table \"" + scopeName + "\">\n\n";
	}
};


class Interpreter {
private:
	const AST::Node* ast;
	ScopedVariableTable globalVariables;
	Value returnValue;

private:
	std::string indent;

public:
	inline Interpreter(const AST::Node* ast): ast(ast), globalVariables("Global Scope"), returnValue() {
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
			case AST::ExpressionNode::Type::LITERAL_EXPRESSION:
				return visitLiteralExpression(scope, dynamic_cast<const AST::LiteralNode*>(node));
			case AST::ExpressionNode::Type::VARIABLE_EXPRESSION:
				return visitVariableExpression(scope, dynamic_cast<const AST::IdentifierNode*>(node));
			case AST::ExpressionNode::Type::UNARY_EXPRESSION:
				return visitUnaryExpression(scope, dynamic_cast<const AST::UnaryExpressionNode*>(node));
			case AST::ExpressionNode::Type::BINARY_EXPRESSION:
				return visitBinaryExpression(scope, dynamic_cast<const AST::BinaryExpressionNode*>(node));
			case AST::ExpressionNode::Type::CALL_EXPRESSION:
				return visitFunctionCall(scope, dynamic_cast<const AST::FunctionCallExpressionNode*>(node));
		}

		throw std::runtime_error("Interpreter::visit(ExpressionNode): invalid expression Node type");
	}

	inline StatementResult visit(ScopedVariableTable* scope, const AST::StatementNode* node) {
		switch(node->type()) {
			case AST::StatementNode::Type::EXPRESSION_STATEMENT:
				return visitExpressionStatement(scope, dynamic_cast<const AST::ExpressionStatement*>(node));
			case AST::StatementNode::Type::STATEMENT_LIST:
				return visitStatementList(scope, dynamic_cast<const AST::StatementList*>(node));
			case AST::StatementNode::Type::RETURN_STATEMENT:
				return visitReturnStatement(scope, dynamic_cast<const AST::ReturnStatement*>(node));

			case AST::StatementNode::Type::IF_STATEMENT:
				return visitIfStatement(scope, dynamic_cast<const AST::IfStatement*>(node));
			case AST::StatementNode::Type::WHILE_STATEMENT:
				return visitWhileStatement(scope, dynamic_cast<const AST::WhileStatement*>(node));
			case AST::StatementNode::Type::FUNCTION_DECLARATION_STATEMENT:
				return visitFunctionDeclaration(scope, dynamic_cast<const AST::FunctionDeclarationStatement*>(node));
			case AST::StatementNode::Type::VARIABLE_DECLARATION_STATEMENT:
				return visitVariableDeclaration(scope, dynamic_cast<const AST::VariableDeclarationStatement*>(node));
			case AST::StatementNode::Type::VARIABLE_ASSIGNMENT_STATEMENT:
				return visitVariableAssignment(scope, dynamic_cast<const AST::VariableAssignmentStatement*>(node));
		}

		throw std::runtime_error("Interpreter::visit(StatementNode): invalid statement Node type");
	}

	Value visitLiteralExpression(ScopedVariableTable* scope, const AST::LiteralNode* node) {
		Value out;

		switch(node->type) {
			case AST::LiteralNode::LiteralType::BOOL:
				// ret = dynamic_cast<const AST::BoolLiteralNode*>(node)->value ? "true" : "false";
				out = Value(dynamic_cast<const AST::BoolLiteralNode*>(node)->value);
				break;
			case AST::LiteralNode::LiteralType::INT:
				// ret = std::to_string(dynamic_cast<const AST::IntLiteralNode*>(node)->value);
				out = Value(dynamic_cast<const AST::IntLiteralNode*>(node)->value);
				break;
			case AST::LiteralNode::LiteralType::FLOAT:
				// ret = std::to_string(dynamic_cast<const AST::FloatLiteralNode*>(node)->value);
				out = Value(dynamic_cast<const AST::FloatLiteralNode*>(node)->value);
				break;
			case AST::LiteralNode::LiteralType::STRING:
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
		std::cout << indent << "<UnaryExpression " << node->opString() << ">:\n";
		
		Value res;
		
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

		indent += "  ";

		const Value va = visit(scope, node->a);

		const Value vb = visit(scope, node->b);

		const std::string& evalType = node->evalType().type();


		Value res;

		#define opCase(T, OP_NAME, OP) \
			case AST::BinaryExpressionNode::Operation::OP_NAME: \
				res = va OP vb; \
				std::cout << "Executed opCase evalType:" #T " opName:" #OP_NAME " op:" #OP "  " << va.toString() << #OP << vb.toString() <<  " -> " << res.toString() << std::endl; \
				break;
		#define typeCase(T) \
		if(evalType == #T) { \
			std::cout << "Executing typeCase " << #T << std::endl; \
			using std::string; \
			switch(node->op) { \
				opCase(T, PLUS, +) \
				opCase(T, MINUS, -) \
				opCase(T, MUL, *) \
				opCase(T, DIV, /) \
				opCase(T, COMP_EQ, ==) \
				opCase(T, COMP_NE, !=) \
				opCase(T, COMP_GT, >) \
				opCase(T, COMP_LT, <) \
				opCase(T, COMP_GE, >=) \
				opCase(T, COMP_LE, <=) \
			} \
		}
		
		const std::string& asdf = "asdf";
		std::cout << "ASD == ASD: " << (asdf == "bool") << std::endl;
		std::cout << "EvalType: <" << evalType << ">  Op: \"" << node->opString() << "\"" << std::endl;
		typeCase(bool)
		std::cout << "After: <" << "bool" << ">" << std::endl;
		typeCase(int)
		std::cout << "After: <" << "int" << ">" << std::endl;
		typeCase(float)
		std::cout << "After: <" << "float" << ">" << std::endl;
		typeCase(string)
		std::cout << "After: <" << "string" << ">" << std::endl;
		#undef typeCase
		#undef opCase

		/*
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
		*/

		if(res.isVoid())
			throw std::runtime_error("Interpreter::visitBinaryExpression: Invalid type or operator in Binary Expression "
				+ node->a->evalType().type()
				+ node->opString()
				+ node->b->evalType().type()
				+ " -> " + node->evalType().type());

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

		const StatementResult out = visit(localScope, targetFunction->body); // TODO: fix warning and rethink
		(void)out;

		indent = indent.substr(0, indent.size() - 2);

		std::cout << indent << "</FunctionCall> => " << returnValue.toString() << "\n";

		localScope->print(indent);

		return returnValue;
	}


	// Stetements:
	StatementResult visitExpressionStatement(ScopedVariableTable* scope, const AST::ExpressionStatement* node) {
		std::cout << indent << "<StatementList>\n";

		indent += "  ";

		visit(scope, node);

		indent = indent.substr(0, indent.size() - 2);

		std::cout << indent << "</StatementList>\n";

		return StatementResult::Void();
	}

	StatementResult visitStatementList(ScopedVariableTable* scope, const AST::StatementList* node) {
		std::cout << indent << "<StatementList>\n";
	
		indent += "  ";

		StatementResult out = StatementResult::Void();

		for(const AST::StatementNode* statement : node->statements) {
			out = visit(scope, statement);

			if(out.type() != StatementResult::Type::VOID) break; // found return, break or continue statement // TODO: fix
		}
		
		indent = indent.substr(0, indent.size() - 2);

		std::cout << indent << "</StatementList>\n";

		return out;
	}

	StatementResult visitReturnStatement(ScopedVariableTable* scope, const AST::ReturnStatement* node) {
		std::cout << indent << "<ReturnStatement>\n";

		indent += "  ";

		returnValue = visit(scope, node->expr);
		// const Value out = visit(scope, node->expr);

		indent = indent.substr(0, indent.size() - 2);

		std::cout << indent << "</ReturnStatement>\n";

		// return out;
		return StatementResult::Return();
	}



	StatementResult visitIfStatement(ScopedVariableTable* scope, const AST::IfStatement* node) {
		std::cout << indent << "<IfStatement>\n";

		indent += "  ";

		const Value cond = visit(scope, node->condition);

		if(!cond.isConvertibleToBool())
			throw std::runtime_error("Interpreter::visitIfStatement: condition not convertible to bool");

		StatementResult res = StatementResult::Void();

		if(cond.to<bool>()) {
			ScopedVariableTable* localScope = new ScopedVariableTable("Local IfStatement Scope", scope);

			res = visit(localScope, node->body);
		}

		indent = indent.substr(0, indent.size() - 2);

		std::cout << indent << "</IfStatement>\n";

		return res; // TODO: fix
	}

	StatementResult visitWhileStatement(ScopedVariableTable* scope, const AST::WhileStatement* node) {
		std::cout << indent << "<WhileStatement>\n";

		indent += "  ";

		// visit(scope, node->expr); // TODO: implement

		indent = indent.substr(0, indent.size() - 2);

		std::cout << indent << "</WhileStatement>\n";

		return StatementResult::Void(); // TODO: fix
	}

	StatementResult visitFunctionDeclaration(ScopedVariableTable* scope, const AST::FunctionDeclarationStatement* node) {
		std::cout << indent << "<FunctionDeclaration/> (skipping)\n";

		return StatementResult::Void();
	}

	StatementResult visitVariableDeclaration(ScopedVariableTable* scope, const AST::VariableDeclarationStatement* node) {
		std::cout << indent << "<VariableDeclaration>\n";

		indent += "  ";

		if(node->initialAssignment)
			visit(scope, node->initialAssignment);

		indent = indent.substr(0, indent.size() - 2);

		std::cout << indent << "</VariableDeclaration>\n";

		return StatementResult::Void();
	}

	StatementResult visitVariableAssignment(ScopedVariableTable* scope, const AST::VariableAssignmentStatement* node) {
		std::cout << indent << "<VariableAssignment \"" + node->varName + "\">\n";

		indent += "  ";

		Value val = visit(scope, node->expr);
		scope->set(node->varName, val);

		indent = indent.substr(0, indent.size() - 2);

		std::cout << indent << "</VariableAssignment>\n";

		return StatementResult::Void();
	}
};

