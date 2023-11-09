#pragma once


#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

#include "ScopedSymbolTable.hpp"
#include "Tokens.hpp"
#include "Types.hpp"


namespace AST {
	static constexpr const char* SPACE =  "  ";     // "  " 
	static constexpr const char* VSPACE = "\xB3 ";  // "│ "
	static constexpr const char* VBRANCH = "\xC3\xC4"; // "├─"
	static constexpr const char* LBRANCH = "\xC0\xC4"; // "└─"
	static constexpr const char* RBRANCH = "\xBF "; // "┐ "
	// 0xB3; // 179 │
	// 0xC0; // 192 └
	// 0xC3; // 195 ├
	// 0xC4; // 196 ─
	// └ ┘ ┌ ┐ │ ─ ┤ ├ ┴ ┬ ┼ 

	inline std::string space(const size_t indent) { std::string res; for(size_t i = 0; i < indent; i++) res += "  "; return res; }

	struct Node {
	public:
		enum class BaseType : uint8_t {
			EXPRESSION, STATEMENT
		};

	protected:
		ScopedSymbolTable* scope;

	private:
		BaseType baseType_;
		Span span_;

	public:
		inline Node(ScopedSymbolTable* scope, const BaseType baseType): scope(scope), baseType_(baseType) {}
		inline virtual ~Node() {}
	
	public:
		inline BaseType baseType() const { return baseType_; }
		inline Span span() const { return span_; }
		inline const ScopedSymbolTable& getScope() const { return *scope; }

	public:
		inline virtual void print(const std::string& indent = "", const bool isLast = true) const = 0;
	};


	struct ExpressionNode : public Node {
	public:
		enum class Type : uint8_t {
			LITERAL_EXPRESSION, VARIABLE_EXPRESSION, UNARY_EXPRESSION, BINARY_EXPRESSION, CALL_EXPRESSION,
		};

	private:
		Type type_;
		EvalType evalType_;

	public:
		inline ExpressionNode(ScopedSymbolTable* scope_, const Type type, const EvalType& evalType): Node(scope_, BaseType::EXPRESSION), type_(type), evalType_(evalType) {}
		inline Type type() const { return type_; }
		inline EvalType evalType() const { return evalType_; }
	};

	struct StatementNode : public Node {
	public:
		enum class Type : uint8_t {
			EXPRESSION_STATEMENT, STATEMENT_LIST, RETURN_STATEMENT,
			IF_STATEMENT, WHILE_STATEMENT, FUNCTION_DECLARATION_STATEMENT, VARIABLE_DECLARATION_STATEMENT, VARIABLE_ASSIGNMENT_STATEMENT,
		};

	private:
		Type type_;
	
	public:
		inline StatementNode(ScopedSymbolTable* scope_, const Type type): Node(scope_, BaseType::STATEMENT), type_(type) {}
		inline Type type() const { return type_; }
	};


	// expressions:
	struct UnaryExpressionNode : public ExpressionNode {
		enum class Operation : uint8_t {
			PLUS, MINUS,
		} op; // operation
		const ExpressionNode *a;

		inline UnaryExpressionNode(ScopedSymbolTable* scope_, const std::string& op_, const ExpressionNode* a):
			ExpressionNode(scope_, Type::UNARY_EXPRESSION, a->evalType()),
			a(a) {
			if(op_ == "+") op = Operation::PLUS;
			else if(op_ == "-") op = Operation::MINUS;
			else throw std::runtime_error("Invalid unary operator");
		}

		inline const char* opString() const {
			switch(op) {
				case Operation::PLUS: return "+";
				case Operation::MINUS: return "-";
			}
			throw std::runtime_error("Invalid unary operator enum value");
		}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << opString();
			std::cout << "    UnaryExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, true);
		}
	};

	struct BinaryExpressionNode : public ExpressionNode {
		enum class Operation : uint8_t {
			PLUS, MINUS, MUL, DIV,

			COMP_EQ, COMP_NE, COMP_GT, COMP_LT, COMP_GE, COMP_LE,
		};
		const ExpressionNode *a;
		Operation op; // operation
		const ExpressionNode *b;

		inline BinaryExpressionNode(ScopedSymbolTable* scope_, const ExpressionNode* a, const std::string& op_, const ExpressionNode* b):
				ExpressionNode(
					scope_,
					Type::BINARY_EXPRESSION,
					binaryExpressionType(a->evalType(), op_, b->evalType())
				),
				a(a), b(b) {
			if(op_ == "+")       op = Operation::PLUS;
			else if(op_ == "-")  op = Operation::MINUS;
			else if(op_ == "*")  op = Operation::MUL;
			else if(op_ == "/")  op = Operation::DIV;
			else if(op_ == "==") op = Operation::COMP_EQ;
			else if(op_ == "!=") op = Operation::COMP_NE;
			else if(op_ == ">")  op = Operation::COMP_GT;
			else if(op_ == "<")  op = Operation::COMP_LT;
			else if(op_ == ">=") op = Operation::COMP_GE;
			else if(op_ == "<=") op = Operation::COMP_LE;
			else throw std::runtime_error("Invalid binary operator");
		}

		inline std::string opString() const {
			switch(op) {
				case Operation::PLUS: return "+";
				case Operation::MINUS: return "-";
				case Operation::MUL: return "*";
				case Operation::DIV: return "/";
				case Operation::COMP_EQ: return "==";
				case Operation::COMP_NE: return "!=";
				case Operation::COMP_GT: return ">";
				case Operation::COMP_LT: return "<";
				case Operation::COMP_GE: return ">=";
				case Operation::COMP_LE: return "<=";
			}
			throw std::runtime_error("Invalid binary operator enum value");
		}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << opString();
			std::cout << "    BinaryExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, false);
			b->print(subIndent, true);
		}
	};

	struct IdentifierNode : public ExpressionNode {
		std::string name;

		inline IdentifierNode(ScopedSymbolTable* scope_, const std::string& name):
			ExpressionNode(
				scope_,
				Type::VARIABLE_EXPRESSION,
				std::get<const std::string>(scope_->lookupRecursive(name)->type)
			),
			name(name) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << "<" << name << ">";
			std::cout << "    Identifier " << span() << "\n";
		}
	};

	struct LiteralNode : public ExpressionNode {
		enum class LiteralType : uint8_t {
			BOOL, INT, FLOAT, STRING
		} type;
		inline LiteralNode(ScopedSymbolTable* scope_, const LiteralType type, const EvalType& evalType):
			ExpressionNode(scope_, Type::LITERAL_EXPRESSION, evalType),
			type(type) {}
	};

	struct BoolLiteralNode : public LiteralNode {
		bool value;

		inline BoolLiteralNode(ScopedSymbolTable* scope_, const bool value):
			LiteralNode(scope_, LiteralNode::LiteralType::BOOL, EvalType("bool")),
			value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << (value ? "true" : "false");
			std::cout << "    BoolLiteral " << span() << "\n";
		}
	};

	struct IntLiteralNode : public LiteralNode {
		int value;

		inline IntLiteralNode(ScopedSymbolTable* scope_, const int value):
			LiteralNode(scope_, LiteralNode::LiteralType::INT, EvalType("int")),
			value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << value;
			std::cout << "    IntLiteral " << span() << "\n";
		}
	};

	struct FloatLiteralNode : public LiteralNode {
		float value;

		inline FloatLiteralNode(ScopedSymbolTable* scope_, const float value):
			LiteralNode(scope_, LiteralNode::LiteralType::FLOAT, EvalType("float")),
			value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << value;
			std::cout << "    FloatLiteral " << span() << "\n";
		}
	};

	struct StringLiteralNode : public LiteralNode {
		std::string value;

		inline StringLiteralNode(ScopedSymbolTable* scope_, const std::string& value):
			LiteralNode(scope_, LiteralNode::LiteralType::STRING, EvalType("string")),
			value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << "\"" << value << "\"";
			std::cout << "    StringLiteral " << span() << "\n";
		}
	};


	// Statements:
	struct VariableAssignmentStatement : public StatementNode {
		std::string varName;
		const ExpressionNode *expr;

		inline VariableAssignmentStatement(ScopedSymbolTable* scope_, const std::string& varName, const ExpressionNode* expr):
			StatementNode(scope_, Type::VARIABLE_ASSIGNMENT_STATEMENT),
			varName(varName), expr(expr) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    Assignment " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << varName << "    Identifier " << "\n";

			expr->print(subIndent, true);
		}
	};

	struct VariableDeclarationStatement : public StatementNode {
		std::string typeName;
		std::string varName;
		const VariableAssignmentStatement* initialAssignment;

		inline VariableDeclarationStatement(ScopedSymbolTable* scope_, const std::string& typeName, const std::string& varName, const VariableAssignmentStatement* initialAssignment):
			StatementNode(scope_, Type::VARIABLE_DECLARATION_STATEMENT),
			typeName(typeName), varName(varName), initialAssignment(initialAssignment) {}
		inline VariableDeclarationStatement(ScopedSymbolTable* scope_, const std::string& typeName, const std::string& varName):
			VariableDeclarationStatement(scope_, typeName, varName, nullptr) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    Declaration " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << typeName << "    Typename " << "\n";
			std::cout << subIndent << (initialAssignment ? VBRANCH : LBRANCH) << varName << "    Identifier " << "\n";

			if(initialAssignment)
				initialAssignment->print(subIndent, true);
		}
	};

	struct ExpressionStatement : public StatementNode {
		const ExpressionNode* expr;

		inline ExpressionStatement(ScopedSymbolTable* scope_, const ExpressionNode* expr):
			StatementNode(scope_, Type::EXPRESSION_STATEMENT),
			expr(expr) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    ExpressionStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			expr->print(subIndent, true);
		}
	};

	struct StatementList : public StatementNode {
		std::vector<const StatementNode*> statements;

		inline StatementList(ScopedSymbolTable* scope_, const std::vector<const StatementNode*>& statements):
			StatementNode(scope_, Type::STATEMENT_LIST),
			statements(statements) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    Block " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			for(const StatementNode* statement : statements)
				statement->print(subIndent, statement == statements.back());
		}
	};

	struct ReturnStatement : public StatementNode {
		const ExpressionNode* expr;

		inline ReturnStatement(ScopedSymbolTable* scope_, const ExpressionNode* expr):
			StatementNode(scope_, Type::RETURN_STATEMENT),
			expr(expr) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    ReturnStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			
			expr->print(subIndent, true);
		}
	};

	struct IfStatement : public StatementNode {
		const ExpressionNode* condition;
		const StatementNode* body;

		inline IfStatement(ScopedSymbolTable* scope_, const ExpressionNode* condition, const StatementNode* body):
			StatementNode(scope_, Type::IF_STATEMENT),
			condition(condition), body(body) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    IfStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			condition->print(subIndent, false);
			body->print(subIndent, true);
		}
	};

	struct WhileStatement : public StatementNode {
		const ExpressionNode* condition;
		const StatementNode* body;

		inline WhileStatement(ScopedSymbolTable* scope_, const ExpressionNode* condition, const StatementNode* body):
			StatementNode(scope_, Type::WHILE_STATEMENT),
			condition(condition), body(body) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    WhileStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			condition->print(subIndent, false);
			body->print(subIndent, true);
		}
	};

	struct FunctionDeclarationStatement : public StatementNode {
		struct Argument { std::string type, name; };

		std::string typeName;
		std::string functionName;
		std::vector<Argument> args;
		const StatementNode* body;

		inline FunctionDeclarationStatement(ScopedSymbolTable* scope_, const std::string& typeName, const std::string& functionName, const std::vector<Argument>& args, const StatementNode* body):
			StatementNode(scope_, Type::FUNCTION_DECLARATION_STATEMENT),
			typeName(typeName), functionName(functionName), args(args), body(body) {}
		
		inline void printArgs(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    ArgumentList " << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			if(args.size() == 0) {
				std::cout << subIndent << LBRANCH << "<empty>\n";
				return;
			}

			for(size_t i = 0; i < args.size(); i++) {
				std::cout << subIndent << VBRANCH << args[i].type << "    Typename " << "\n";
				std::cout << subIndent << (i==args.size()-1 ? LBRANCH : VBRANCH) << args[i].name << "    Identifier " << "\n";
			}
		}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    FunctionDeclarationStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << typeName << "    Typename " << "\n";
			std::cout << subIndent << VBRANCH << functionName << "    Identifier " << "\n";

			printArgs(subIndent, false);
			body->print(subIndent, true);
		}
	};

	struct FunctionCallExpressionNode : public ExpressionNode {
		std::string name; // function name
		std::vector<const ExpressionNode*> args; // function call arguments

		inline FunctionCallExpressionNode(ScopedSymbolTable* scope_, const std::string& name, const std::vector<const ExpressionNode*>& args):
			ExpressionNode(
				scope_,
				Type::CALL_EXPRESSION,
				EvalType(
					dynamic_cast<const FunctionDeclarationStatement*>(
						std::get<const Node*>(scope_->lookupRecursive(name)->type)
					)->typeName
				)
			),
			name(name), args(args) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    FunctionCall " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << name << "    Identifier " << "\n";
			for(const ExpressionNode* arg : args)
				arg->print(subIndent, arg == args.back());
		}
	};
};
