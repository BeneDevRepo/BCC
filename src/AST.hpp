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
		inline Span span() const { return span_; };
	};


	struct ExpressionNode : public Node { };

	struct StatementNode : public Node { };


	// expressions:

	// struct FunctionCallExpressionNode : public ExpressionNode {
	// 	Token name; // function name
	// 	Token openParen;
	// 	std::vector<ExpressionNode*> args; // function call arguments
	// 	std::vector<Token> commas; // commas between function call arguments
	// 	Token closeParen;

	// 	inline FunctionCallExpressionNode(const Token& name, const Token& openParen, const std::vector<ExpressionNode*>& args, const std::vector<Token>& commas, const Token& closeParen):
	// 		name(name), openParen(openParen), args(args), commas(commas), closeParen(closeParen) {}

	// 	inline virtual void print(const std::string& indent, const bool isLast) const {
	// 		std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
	// 		std::cout << RBRANCH << "    FunctionCall " << span() << "\n";

	// 		const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
	// 		std::cout << subIndent << VBRANCH << name.value << "    Identifier " << name.span << "\n";
	// 		std::cout << subIndent << VBRANCH << openParen.value << "    OpenParen " << openParen.span << "\n";
	// 		for(const ExpressionNode* arg : args)
	// 			arg->print(subIndent, false);
	// 		std::cout << subIndent << LBRANCH << closeParen.value << "    CloseParen " << closeParen.span << "\n";
	// 	}
	// };

	struct UnaryExpressionNode : public ExpressionNode {
		char op; // operation
		ExpressionNode *a;

		inline UnaryExpressionNode(ExpressionNode* a, const char op): op(op), a(a) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << op;
			std::cout << "    UnaryExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, true);
		}
	};

	struct BinaryExpressionNode : public ExpressionNode {
		ExpressionNode *a, *b;
		Token op; // operation

		inline BinaryExpressionNode(ExpressionNode* a, ExpressionNode* b, const Token& op): a(a), b(b), op(op) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << op.value;
			std::cout << "    BinaryExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, false);
			b->print(subIndent, true);
		}
	};

	struct IdentifierNode : public ExpressionNode {
		Token name;

		inline IdentifierNode(const Token& name): name(name) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << "<" << name.value << ">";
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
		Token typeName;
		IdentifierNode *varName;
		Token equals;
		ExpressionNode *expr;
		Token semicolon;

		inline VariableDeclarationStatement(const Token& typeName, IdentifierNode* varName, const Token& semicolon):
			typeName(typeName), varName(varName), equals(), expr(nullptr), semicolon(semicolon) {}
		inline VariableDeclarationStatement(const Token& typeName, IdentifierNode* varName, const Token& equals, ExpressionNode* expr, const Token& semicolon):
			typeName(typeName), varName(varName), equals(equals), expr(expr), semicolon(semicolon) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    Declaration " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << typeName.value << "    Typename " << typeName.span << "\n";
			varName->print(subIndent, false);

			if(expr) {
				std::cout << subIndent << VBRANCH << equals.value << "    Operator " << equals.span << "\n";
				expr->print(subIndent, false);
			}

			std::cout << subIndent << LBRANCH << semicolon.value << "    Semicolon " << span() << "\n";
		}
	};

	struct ExpressionStatement : public StatementNode {
		ExpressionNode* expr;
		Token semicolon;

		inline ExpressionStatement(ExpressionNode* expr, const Token& semicolon): expr(expr), semicolon(semicolon) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    ExpressionStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			expr->print(subIndent, false);

			std::cout << subIndent << LBRANCH << semicolon.value << "    Semicolon " << semicolon.span << "\n";
		}
	};

	struct BlockStatement : public StatementNode {
		Token openBrace;
		std::vector<StatementNode*> statements;
		Token closeBrace;

		inline BlockStatement(const Token& openBrace, const std::vector<StatementNode*>& statements, const Token& closeBrace): openBrace(openBrace), statements(statements), closeBrace(closeBrace) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    Block " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << openBrace.value << "    OpenBrace " << openBrace.span << "\n";

			for(StatementNode* statement : statements)
				statement->print(subIndent, false);

			std::cout << subIndent << LBRANCH << closeBrace.value << "    CloseBrace " << closeBrace.span << "\n";
		}
	};

	struct ReturnStatement : public StatementNode {
		Token returnToken;
		ExpressionNode* expr;
		Token semicolon;

		inline ReturnStatement(const Token& returnToken, ExpressionNode* expr, const Token& semicolon):
			returnToken(returnToken), expr(expr), semicolon(semicolon) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    IfStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << returnToken.value << "    ReturnKeyword " << returnToken.span << "\n";
			expr->print(subIndent, false);
			std::cout << subIndent << LBRANCH << semicolon.value << "    semicolon " << semicolon.span << "\n";
		}
	};

	struct IfStatement : public StatementNode {
		Token ifToken;
		Token openParen;
		ExpressionNode* condition;
		Token closeParen;
		StatementNode* body;

		inline IfStatement(const Token& ifToken, const Token& openParen, ExpressionNode* condition, const Token& closeParen, StatementNode* body):
			ifToken(ifToken), openParen(openParen), condition(condition), closeParen(closeParen), body(body) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    IfStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << ifToken.value << "    IfKeyword " << ifToken.span << "\n";
			std::cout << subIndent << VBRANCH << openParen.value << "    OpenParen " << openParen.span << "\n";
			condition->print(subIndent, false);
			std::cout << subIndent << VBRANCH << closeParen.value << "    CloseParen " << closeParen.span << "\n";
			body->print(subIndent, true);
		}
	};

	struct WhileStatement : public StatementNode {
		Token whileToken;
		Token openParen;
		ExpressionNode* condition;
		Token closeParen;
		StatementNode* body;

		inline WhileStatement(const Token& whileToken, const Token& openParen, ExpressionNode* condition, const Token& closeParen, StatementNode* body):
			whileToken(whileToken), openParen(openParen), condition(condition), closeParen(closeParen), body(body) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    WhileStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << whileToken.value << "    WhileKeyword " << whileToken.span << "\n";
			std::cout << subIndent << VBRANCH << openParen.value << "    OpenParen " << openParen.span << "\n";
			condition->print(subIndent, false);
			std::cout << subIndent << VBRANCH << closeParen.value << "    CloseParen " << closeParen.span << "\n";
			body->print(subIndent, true);
		}
	};

	struct ArgumentsNode : public ExpressionNode {
		struct Argument { Token type, name; };
		std::vector<Argument> args;
		std::vector<Token> commas;

		inline ArgumentsNode(const std::vector<Argument>& args, const std::vector<Token>& commas): args(args), commas(commas) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    ArgumentList " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			if(args.size() == 0) {
				std::cout << subIndent << LBRANCH << "<empty>\n";
				return;
			}

			for(size_t i = 0; i < commas.size(); i++) {
				std::cout << subIndent << VBRANCH << args[i].type.value << "    Typename " << args[i].type.span << "\n";
				std::cout << subIndent << VBRANCH << args[i].name.value << "    Identifier " << args[i].name.span << "\n";
				std::cout << subIndent << VBRANCH << commas[i].value << "    Comma " << commas[i].span << "\n";
			}

			std::cout << subIndent << VBRANCH << args.back().type.value << "    Typename " << args.back().type.span << "\n";
			std::cout << subIndent << LBRANCH << args.back().name.value << "    Identifier " << args.back().name.span << "\n";
		}
	};

	struct FunctionDeclarationStatement : public StatementNode {
		Token typeName;
		Token functionName;
		Token openParen;
		ArgumentsNode* args;
		Token closeParen;
		StatementNode* body;

		inline FunctionDeclarationStatement(const Token& typeName, const Token& functionName, const Token& openParen, ArgumentsNode* args, const Token& closeParen, StatementNode* body):
			typeName(typeName), functionName(functionName), openParen(openParen), args(args), closeParen(closeParen), body(body) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    FunctionDeclarationStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << typeName.value << "    Typename " << typeName.span << "\n";
			std::cout << subIndent << VBRANCH << functionName.value << "    Identifier " << functionName.span << "\n";
			std::cout << subIndent << VBRANCH << openParen.value << "    OpenParen " << openParen.span << "\n";
			args->print(subIndent, false);
			std::cout << subIndent << VBRANCH << closeParen.value << "    CloseParen " << closeParen.span << "\n";
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