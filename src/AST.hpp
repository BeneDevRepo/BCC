#pragma once


#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

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
	private:
		Span span_;
	public:
		// inline Node(const Span& span_): span_(span_) {}
		inline virtual ~Node() {}
		inline virtual void print(const std::string& indent = "", const bool isLast = true) const = 0;
		inline Span span() const { return span_; }
	};


	struct ExpressionNode : public Node { };

	struct StatementNode : public Node { };


	// expressions:

	struct FunctionCallExpressionNode : public ExpressionNode {
		std::string name; // function name
		std::vector<ExpressionNode*> args; // function call arguments

		inline FunctionCallExpressionNode(const std::string& name, const std::vector<ExpressionNode*>& args):
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

	struct UnaryExpressionNode : public ExpressionNode {
		char op; // operation
		ExpressionNode *a;

		inline UnaryExpressionNode(const char op, ExpressionNode* a): op(op), a(a) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << op;
			std::cout << "    UnaryExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, true);
		}
	};

	struct BinaryExpressionNode : public ExpressionNode {
		ExpressionNode *a;
		char op; // operation
		ExpressionNode *b;

		inline BinaryExpressionNode(ExpressionNode* a, const char op, ExpressionNode* b): a(a), op(op), b(b) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << op;
			std::cout << "    BinaryExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, false);
			b->print(subIndent, true);
		}
	};

	struct IdentifierNode : public ExpressionNode {
		std::string name;

		inline IdentifierNode(const std::string& name): name(name) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << "<" << name << ">";
			std::cout << "    Identifier " << span() << "\n";
		}
	};

	struct LiteralNode : public ExpressionNode {
		enum class Type : uint8_t {
			BOOL, INT, FLOAT, STRING
		} type;
		inline LiteralNode(const Type type): type(type) {}
	};

	struct BoolLiteralNode : public LiteralNode {
		bool value;

		inline BoolLiteralNode(const bool value): LiteralNode(LiteralNode::Type::BOOL), value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << (value ? "true" : "false");
			std::cout << "    BoolLiteral " << span() << "\n";
		}
	};

	struct IntLiteralNode : public LiteralNode {
		int value;

		inline IntLiteralNode(const int value): LiteralNode(LiteralNode::Type::INT), value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << value;
			std::cout << "    IntLiteral " << span() << "\n";
		}
	};

	struct FloatLiteralNode : public LiteralNode {
		float value;

		inline FloatLiteralNode(const float value): LiteralNode(LiteralNode::Type::FLOAT), value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << value;
			std::cout << "    FloatLiteral " << span() << "\n";
		}
	};

	struct StringLiteralNode : public LiteralNode {
		std::string value;

		inline StringLiteralNode(const std::string& value): LiteralNode(LiteralNode::Type::STRING), value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << value;
			std::cout << "    StringLiteral " << span() << "\n";
		}
	};


	// Statements:
	struct VariableDeclarationStatement : public StatementNode {
		std::string typeName;
		std::string varName;

		inline VariableDeclarationStatement(const std::string& typeName, const std::string& varName):
			typeName(typeName), varName(varName) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    Declaration " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << typeName << "    Typename " << "\n";
			std::cout << subIndent << LBRANCH << varName << "    Identifier " << "\n";
		}
	};

	struct VariableAssignmentStatement : public StatementNode {
		std::string varName;
		ExpressionNode *expr;

		inline VariableAssignmentStatement(const std::string& varName, ExpressionNode* expr):
			varName(varName), expr(expr) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    Assignment " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << varName << "    Identifier " << "\n";

			expr->print(subIndent, true);
		}
	};

	struct ExpressionStatement : public StatementNode {
		ExpressionNode* expr;

		inline ExpressionStatement(ExpressionNode* expr): expr(expr) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    ExpressionStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			expr->print(subIndent, true);
		}
	};

	struct StatementList : public StatementNode {
		std::vector<StatementNode*> statements;

		inline StatementList(const std::vector<StatementNode*>& statements): statements(statements) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    Block " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			for(StatementNode* statement : statements)
				statement->print(subIndent, statement == statements.back());
		}
	};

	struct ReturnStatement : public StatementNode {
		ExpressionNode* expr;

		inline ReturnStatement(ExpressionNode* expr): expr(expr) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    ReturnStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			
			expr->print(subIndent, true);
		}
	};

	struct IfStatement : public StatementNode {
		ExpressionNode* condition;
		StatementNode* body;

		inline IfStatement(ExpressionNode* condition, StatementNode* body):
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
		ExpressionNode* condition;
		StatementNode* body;

		inline WhileStatement(ExpressionNode* condition, StatementNode* body):
			condition(condition), body(body) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    WhileStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			condition->print(subIndent, false);
			body->print(subIndent, true);
		}
	};

	struct ArgumentsNode : public ExpressionNode {
		struct Argument { std::string type, name; };
		std::vector<Argument> args;

		inline ArgumentsNode(const std::vector<Argument>& args): args(args) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    ArgumentList " << span() << "\n";

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
		ArgumentsNode* args;
		StatementNode* body;

		inline FunctionDeclarationStatement(const std::string& typeName, const std::string& functionName, ArgumentsNode* args, StatementNode* body):
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
	};

	// Program:
	struct Program : public Node {
		std::vector<StatementNode*> statements;

		inline Program(const std::vector<StatementNode*>& statements): statements(statements) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    Program " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			for(StatementNode* statement : statements)
				statement->print(subIndent, statement == statements.back());
		}
	};
};
