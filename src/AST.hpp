#pragma once


#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

#include "SymbolTable.hpp"
#include "Tokens.hpp"


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
		enum class Type : uint8_t {
			LITERAL_EXPRESSION, VARIABLE_EXPRESSION, UNARY_EXPRESSION, BINARY_EXPRESSION, CALL_EXPRESSION, 

			EXPRESSION_STATEMENT, STATEMENT_LIST, RETURN_STATEMENT,
			IF_STATEMENT, WHILE_STATEMENT, FUNCTION_DECLARATION_STATEMENT, VARIABLE_DECLARATION_STATEMENT, VARIABLE_ASSIGNMENT_STATEMENT,
		};
	private:
		Type type_;
		Span span_;
	public:
		inline Node(const Type type): type_(type) {}
		inline virtual ~Node() {}
		inline virtual void print(const std::string& indent = "", const bool isLast = true) const = 0;
		inline virtual void visit(SymbolTable& symbols) const = 0;
		inline Type type() const { return type_; }
		inline Span span() const { return span_; }
	};


	struct ExpressionNode : public Node {
		inline ExpressionNode(const Node::Type type): Node(type) {}
	};

	struct StatementNode : public Node {
		inline StatementNode(const Node::Type type): Node(type) {}
	};


	// expressions:
	struct FunctionCallExpressionNode : public ExpressionNode {
		std::string name; // function name
		std::vector<const ExpressionNode*> args; // function call arguments

		inline FunctionCallExpressionNode(const std::string& name, const std::vector<const ExpressionNode*>& args):
			ExpressionNode(Node::Type::CALL_EXPRESSION),
			name(name), args(args) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    FunctionCall " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << name << "    Identifier " << "\n";
			for(const ExpressionNode* arg : args)
				arg->print(subIndent, arg == args.back());
		}

		inline virtual void visit(SymbolTable& symbols) const {
			if(!symbols.lookup(name))
				throw std::runtime_error("Tried to call unknown function \"" + name + "\"");

			if(symbols.lookup(name)->category != Symbol::Category::FUNCTION)
				throw std::runtime_error("Symbol \"" + name + "\" in Function call expression does not refer to a function.");
			
			for(const ExpressionNode* arg : args)
				arg->visit(symbols);
		}
	};

	struct UnaryExpressionNode : public ExpressionNode {
		char op; // operation
		const ExpressionNode *a;

		inline UnaryExpressionNode(const char op, const ExpressionNode* a):
			ExpressionNode(Node::Type::UNARY_EXPRESSION),
			op(op), a(a) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << op;
			std::cout << "    UnaryExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, true);
		}

		inline virtual void visit(SymbolTable& symbols) const {
			a->visit(symbols);
		}
	};

	struct BinaryExpressionNode : public ExpressionNode {
		const ExpressionNode *a;
		char op; // operation
		const ExpressionNode *b;

		inline BinaryExpressionNode(const ExpressionNode* a, const char op, const ExpressionNode* b):
			ExpressionNode(Node::Type::BINARY_EXPRESSION),
			a(a), op(op), b(b) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << op;
			std::cout << "    BinaryExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, false);
			b->print(subIndent, true);
		}

		inline virtual void visit(SymbolTable& symbols) const {
			a->visit(symbols);
			b->visit(symbols);
		}
	};

	struct IdentifierNode : public ExpressionNode {
		std::string name;

		inline IdentifierNode(const std::string& name):
			ExpressionNode(Node::Type::VARIABLE_EXPRESSION),
			name(name) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << "<" << name << ">";
			std::cout << "    Identifier " << span() << "\n";
		}

		inline virtual void visit(SymbolTable& symbols) const {
			if(!symbols.lookup(name))
				throw std::runtime_error("Use of undeclared identifier \"" + name + "\"");
		}
	};

	struct LiteralNode : public ExpressionNode {
		enum class Type : uint8_t {
			BOOL, INT, FLOAT, STRING
		} type;
		inline LiteralNode(const Type type):
			ExpressionNode(Node::Type::LITERAL_EXPRESSION),
			type(type) {}
	};

	struct BoolLiteralNode : public LiteralNode {
		bool value;

		inline BoolLiteralNode(const bool value): LiteralNode(LiteralNode::Type::BOOL), value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << (value ? "true" : "false");
			std::cout << "    BoolLiteral " << span() << "\n";
		}

		inline virtual void visit(SymbolTable& symbols) const {
		}
	};

	struct IntLiteralNode : public LiteralNode {
		int value;

		inline IntLiteralNode(const int value): LiteralNode(LiteralNode::Type::INT), value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << value;
			std::cout << "    IntLiteral " << span() << "\n";
		}

		inline virtual void visit(SymbolTable& symbols) const {
		}
	};

	struct FloatLiteralNode : public LiteralNode {
		float value;

		inline FloatLiteralNode(const float value): LiteralNode(LiteralNode::Type::FLOAT), value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << value;
			std::cout << "    FloatLiteral " << span() << "\n";
		}

		inline virtual void visit(SymbolTable& symbols) const {
		}
	};

	struct StringLiteralNode : public LiteralNode {
		std::string value;

		inline StringLiteralNode(const std::string& value): LiteralNode(LiteralNode::Type::STRING), value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << value;
			std::cout << "    StringLiteral " << span() << "\n";
		}

		inline virtual void visit(SymbolTable& symbols) const {
		}
	};


	// Statements:
	struct VariableAssignmentStatement : public StatementNode {
		std::string varName;
		const ExpressionNode *expr;

		inline VariableAssignmentStatement(const std::string& varName, const ExpressionNode* expr):
			StatementNode(Node::Type::VARIABLE_ASSIGNMENT_STATEMENT),
			varName(varName), expr(expr) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    Assignment " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << varName << "    Identifier " << "\n";

			expr->print(subIndent, true);
		}

		inline virtual void visit(SymbolTable& symbols) const {
			if(!symbols.lookup(varName))
				throw std::runtime_error("Assignment to undeclared Variable \"" + varName + "\"");
			
			expr->visit(symbols);
		}
	};

	struct VariableDeclarationStatement : public StatementNode {
		std::string typeName;
		std::string varName;
		const VariableAssignmentStatement* initialAssignment;

		inline VariableDeclarationStatement(const std::string& typeName, const std::string& varName, const VariableAssignmentStatement* initialAssignment):
			StatementNode(Node::Type::VARIABLE_DECLARATION_STATEMENT),
			typeName(typeName), varName(varName), initialAssignment(initialAssignment) {}
		inline VariableDeclarationStatement(const std::string& typeName, const std::string& varName): VariableDeclarationStatement(typeName, varName, nullptr) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    Declaration " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << typeName << "    Typename " << "\n";
			std::cout << subIndent << (initialAssignment ? VBRANCH : LBRANCH) << varName << "    Identifier " << "\n";

			if(initialAssignment)
				initialAssignment->print(subIndent, true);
		}

		inline virtual void visit(SymbolTable& symbols) const {
			if(!symbols.lookup(typeName))
				throw std::runtime_error("Unknown typename \"" + typeName + "\" in declaration of \"" + varName + "\"");
			if(symbols.lookup(varName))
				throw std::runtime_error("Redeclaration of symbol \"" + varName + "\" in variable declaration");

			symbols.declare(new Symbol(Symbol::Category::VARIABLE, varName, typeName));

			if(initialAssignment)
				initialAssignment->visit(symbols);
		}
	};

	struct ExpressionStatement : public StatementNode {
		const ExpressionNode* expr;

		inline ExpressionStatement(const ExpressionNode* expr):
			StatementNode(Node::Type::EXPRESSION_STATEMENT),
			expr(expr) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    ExpressionStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			expr->print(subIndent, true);
		}

		inline virtual void visit(SymbolTable& symbols) const {
			expr->visit(symbols);
		}
	};

	struct StatementList : public StatementNode {
		std::vector<const StatementNode*> statements;

		inline StatementList(const std::vector<const StatementNode*>& statements):
			StatementNode(Node::Type::STATEMENT_LIST),
			statements(statements) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    Block " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			for(const StatementNode* statement : statements)
				statement->print(subIndent, statement == statements.back());
		}

		inline virtual void visit(SymbolTable& symbols) const {
			for(const StatementNode* statement : statements)
				statement->visit(symbols);
		}
	};

	struct ReturnStatement : public StatementNode {
		const ExpressionNode* expr;

		inline ReturnStatement(const ExpressionNode* expr):
			StatementNode(Node::Type::RETURN_STATEMENT),
			expr(expr) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    ReturnStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			
			expr->print(subIndent, true);
		}

		inline virtual void visit(SymbolTable& symbols) const {
			expr->visit(symbols);
		}
	};

	struct IfStatement : public StatementNode {
		const ExpressionNode* condition;
		const StatementNode* body;

		inline IfStatement(const ExpressionNode* condition, const StatementNode* body):
			StatementNode(Node::Type::IF_STATEMENT),
			condition(condition), body(body) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    IfStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			condition->print(subIndent, false);
			body->print(subIndent, true);
		}

		inline virtual void visit(SymbolTable& symbols) const {
			condition->visit(symbols);
			body->visit(symbols);
		}
	};

	struct WhileStatement : public StatementNode {
		const ExpressionNode* condition;
		const StatementNode* body;

		inline WhileStatement(const ExpressionNode* condition, const StatementNode* body):
			StatementNode(Node::Type::WHILE_STATEMENT),
			condition(condition), body(body) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    WhileStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			condition->print(subIndent, false);
			body->print(subIndent, true);
		}

		inline virtual void visit(SymbolTable& symbols) const {
			condition->visit(symbols);
			body->visit(symbols);
		}
	};

	struct ArgumentsNode {
		struct Argument { std::string type, name; };
		std::vector<Argument> args;

		inline ArgumentsNode(const std::vector<Argument>& args):
			args(args) {}

		inline void print(const std::string& indent, const bool isLast) const {
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
	};

	struct FunctionDeclarationStatement : public StatementNode {
		std::string typeName;
		std::string functionName;
		const ArgumentsNode* args;
		const StatementNode* body;

		inline FunctionDeclarationStatement(const std::string& typeName, const std::string& functionName, const ArgumentsNode* args, const StatementNode* body):
			StatementNode(Node::Type::FUNCTION_DECLARATION_STATEMENT),
			typeName(typeName), functionName(functionName), args(args), body(body) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    FunctionDeclarationStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << typeName << "    Typename " << "\n";
			std::cout << subIndent << VBRANCH << functionName << "    Identifier " << "\n";
			args->print(subIndent, false);
			body->print(subIndent, true);
		}

		inline virtual void visit(SymbolTable& symbols) const {
			if(!symbols.lookup(typeName))
				throw std::runtime_error("Error declaring function: Unknown return type \"" + typeName + "\"");
			if(symbols.lookup(functionName))
				throw std::runtime_error("Error declaring function: Redeclaration of symbol \"" + functionName + "\"");
			
			symbols.declare(new Symbol(Symbol::Category::FUNCTION, functionName, "_userFunction"));
			
			body->visit(symbols);
		}
	};

	// // Program:
	// struct Program : public Node {
	// 	std::vector<StatementNode*> statements;

	// 	inline Program(const std::vector<StatementNode*>& statements): statements(statements) {}

	// 	inline virtual void print(const std::string& indent, const bool isLast) const {
	// 		std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

	// 		std::cout << RBRANCH << "    Program " << span() << "\n";

	// 		const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

	// 		for(StatementNode* statement : statements)
	// 			statement->print(subIndent, statement == statements.back());
	// 	}
	// };
};
