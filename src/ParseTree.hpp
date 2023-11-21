#pragma once


#include <stdexcept>
#include <ostream>
#include <cstdint>
#include <string>
#include <vector>

#include "Tokens.hpp"


namespace ParseTree {
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
			EXPRESSION, STATEMENT, PROGRAM
		};

	private:
		BaseType baseType_;

	public:
		inline Node(const BaseType baseType): baseType_(baseType) {}
		inline virtual ~Node() {}

	public:
		inline BaseType baseType() const { return baseType_; }
	
	public:
		// inline virtual void print(const std::string& indent = "", const bool isLast = true) const = 0;
		inline virtual void print(std::ostream& console, const std::string& indent = "", const bool isLast = true) const = 0;
		inline virtual Span span() const = 0;
		inline virtual std::string toString(const size_t indent = 0) const = 0; // reconstructs source code
	};


	struct ExpressionNode : public Node {
	public:
		enum class Type : uint8_t {
			LITERAL_EXPRESSION, VARIABLE_EXPRESSION, UNARY_EXPRESSION, BINARY_EXPRESSION, CALL_EXPRESSION, GROUP_EXPRESSION,
		};

	private:
		Type type_;

	public:
		inline ExpressionNode(const Type type): Node(BaseType::EXPRESSION), type_(type) {}
		inline Type type() const { return type_; }
	};


	struct StatementNode : public Node {
	public:
		enum class Type : uint8_t {
			EXPRESSION_STATEMENT, BLOCK_STATEMENT, RETURN_STATEMENT,
			IF_STATEMENT, WHILE_STATEMENT, FUNCTION_DECLARATION, VARIABLE_DECLARATION, VARIABLE_ASSIGNMENT,
		};

	private:
		Type type_;

	public:
		inline StatementNode(const Type type): Node(BaseType::STATEMENT), type_(type) {}
		inline Type type() const { return type_; }
	};


	// expressions:
	struct FunctionCallExpressionNode : public ExpressionNode {
		Token name; // function name
		Token openParen;
		std::vector<const ExpressionNode*> args; // function call arguments
		std::vector<Token> commas; // commas between function call arguments
		Token closeParen;

		inline FunctionCallExpressionNode(const Token& name, const Token& openParen, const std::vector<const ExpressionNode*>& args, const std::vector<Token>& commas, const Token& closeParen):
			ExpressionNode(Type::CALL_EXPRESSION),
			name(name), openParen(openParen), args(args), commas(commas), closeParen(closeParen) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			console << RBRANCH << "    FunctionCall " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			console << subIndent << VBRANCH << name.value << "    Identifier " << name.span << "\n";
			console << subIndent << VBRANCH << openParen.value << "    OpenParen " << openParen.span << "\n";
			for(const ExpressionNode* arg : args)
				arg->print(console, subIndent, false);
			console << subIndent << LBRANCH << closeParen.value << "    CloseParen " << closeParen.span << "\n";
		}

		inline virtual Span span() const override { return Span(name.span, closeParen.span); }

		inline virtual std::string toString(const size_t indent) const override {
			std::string res = space(indent) + name.value + openParen.value;
			for(size_t i = 0; i < commas.size(); i++)
				res += args[i]->toString(0) + commas[i].value + " ";
			if(args.size() > 0)
				res += args.back()->toString(0);
			res += closeParen.value;
			return res;
		}
	};

	struct GroupExpressionNode : public ExpressionNode {
		Token openParen;
		const ExpressionNode *a;
		Token closeParen;

		inline GroupExpressionNode(const Token& openParen, const ExpressionNode* a, const Token& closeParen):
			ExpressionNode(Type::GROUP_EXPRESSION),
			openParen(openParen), a(a), closeParen(closeParen) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			console << RBRANCH;
			console << "    GroupExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			console << subIndent << VBRANCH << openParen.value << "\n";
			a->print(console, subIndent, false);
			console << subIndent << LBRANCH << closeParen.value << "\n";
		}

		inline virtual Span span() const override { return Span(openParen.span, closeParen.span); }

		inline virtual std::string toString(const size_t indent) const override { return space(indent) + openParen.value + a->toString(0) + closeParen.value; }
	};

	struct UnaryExpressionNode : public ExpressionNode {
		Token op; // operation
		const ExpressionNode *a;

		inline UnaryExpressionNode(const Token& op, const ExpressionNode* a):
			ExpressionNode(Type::UNARY_EXPRESSION),
			op(op), a(a) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			console << op.value;
			console << "    UnaryExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(console, subIndent, true);
		}

		inline virtual Span span() const override { return Span(a->span(), op.span); }

		inline virtual std::string toString(const size_t indent) const override { return space(indent) + op.value + a->toString(0); }
	};

	struct BinaryExpressionNode : public ExpressionNode {
		const ExpressionNode *a;
		Token op; // operation
		const ExpressionNode *b;

		inline BinaryExpressionNode(const ExpressionNode* a, const Token& op, const ExpressionNode* b):
			ExpressionNode(Type::BINARY_EXPRESSION),
			a(a), op(op), b(b) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			console << op.value;
			console << "    BinaryExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(console, subIndent, false);
			b->print(console, subIndent, true);
		}

		inline virtual Span span() const override { return Span(a->span(), b->span()); }

		inline virtual std::string toString(const size_t indent) const override { return space(indent) + a->toString(0) + " " + op.value + " " + b->toString(0); }
	};

	struct IdentifierNode : public ExpressionNode {
		Token name;

		inline IdentifierNode(const Token& name):
			ExpressionNode(Type::VARIABLE_EXPRESSION),
			name(name) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH) << "<" << name.value << ">";
			console << "    Identifier " << span() << "\n";
		}

		inline virtual Span span() const override { return name.span; }

		inline virtual std::string toString(const size_t indent) const override { return space(indent) + name.value; }
	};

	struct LiteralNode : public ExpressionNode {
		Token value;

		inline LiteralNode(const Token& value):
			ExpressionNode(Type::LITERAL_EXPRESSION),
			value(value) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH) << value.value;
			console << "    Literal " << span() << "\n";
		}

		inline virtual Span span() const override { return value.span; }

		inline virtual std::string toString(const size_t indent) const override { return space(indent) + value.value; }
	};


	// Statements:
	struct VariableDeclarationStatement : public StatementNode {
		Token typeName;
		Token varName;
		Token equals;
		const ExpressionNode* expr;
		Token semicolon;

		inline VariableDeclarationStatement(const Token& typeName, const Token& varName, const Token& equals, const ExpressionNode* expr, const Token& semicolon):
			StatementNode(Type::VARIABLE_DECLARATION),
			typeName(typeName), varName(varName), equals(equals), expr(expr), semicolon(semicolon) {}
		inline VariableDeclarationStatement(const Token& typeName, const Token& varName, const Token& semicolon):
			VariableDeclarationStatement(typeName, varName, equals, nullptr, semicolon) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			console << RBRANCH << "    Declaration " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			console << subIndent << VBRANCH << typeName.value << "    Typename " << typeName.span << "\n";

			console << subIndent << VBRANCH << varName.value << "    Identifier " << varName.span << "\n";

			if(expr) {
				console << subIndent << VBRANCH << equals.value << "    Operator " << equals.span << "\n";
				expr->print(console, subIndent, false);
			}

			console << subIndent << LBRANCH << semicolon.value << "    Semicolon " << span() << "\n";
		}

		inline virtual Span span() const override { return Span(typeName.span, semicolon.span); }

		inline virtual std::string toString(const size_t indent) const override { return space(indent) + typeName.value + " " + varName.value + (expr ? (" " + equals.value + " " + expr->toString(0)) : "") + semicolon.value; }
	};

	struct VariableAssignmentStatement : public StatementNode {
		Token varName;
		Token equals;
		const ExpressionNode* expr;
		Token semicolon;

		inline VariableAssignmentStatement(const Token& varName, const Token& equals, const ExpressionNode* expr, const Token& semicolon):
			StatementNode(Type::VARIABLE_ASSIGNMENT),
			varName(varName), equals(equals), expr(expr), semicolon(semicolon) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			console << RBRANCH << "    Assignment " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			console << subIndent << VBRANCH << varName.value << "    Identifier " << varName.span << "\n";

			if(expr) {
				console << subIndent << VBRANCH << equals.value << "    Operator " << equals.span << "\n";
				expr->print(console, subIndent, false);
			}

			console << subIndent << LBRANCH << semicolon.value << "    Semicolon " << span() << "\n";
		}

		inline virtual Span span() const override { return Span(varName.span, semicolon.span); }

		inline virtual std::string toString(const size_t indent) const override { return space(indent) + varName.value + (expr ? (" " + equals.value + " " + expr->toString(0)) : "") + semicolon.value; }
	};

	struct ExpressionStatement : public StatementNode {
		const ExpressionNode* expr;
		Token semicolon;

		inline ExpressionStatement(const ExpressionNode* expr, const Token& semicolon):
			StatementNode(Type::EXPRESSION_STATEMENT),
			expr(expr), semicolon(semicolon) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			console << RBRANCH << "    ExpressionStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			expr->print(console, subIndent, false);

			console << subIndent << LBRANCH << semicolon.value << "    Semicolon " << semicolon.span << "\n";
		}

		inline virtual Span span() const override { return Span(expr->span(), semicolon.span); }

		inline virtual std::string toString(const size_t indent) const override {
			return expr->toString(indent) + semicolon.value;
		}
	};

	struct BlockStatement : public StatementNode {
		Token openBrace;
		std::vector<const StatementNode*> statements;
		Token closeBrace;
		mutable bool createScope;

		inline BlockStatement(const Token& openBrace, const std::vector<const StatementNode*>& statements, const Token& closeBrace):
			StatementNode(Type::BLOCK_STATEMENT),
			openBrace(openBrace), statements(statements), closeBrace(closeBrace), createScope(true) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			console << RBRANCH << "    Block " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			console << subIndent << VBRANCH << openBrace.value << "    OpenBrace " << openBrace.span << "\n";

			for(const StatementNode* statement : statements)
				statement->print(console, subIndent, false);

			console << subIndent << LBRANCH << closeBrace.value << "    CloseBrace " << closeBrace.span << "\n";
		}

		inline virtual Span span() const override { return Span(openBrace.span, closeBrace.span); }

		inline virtual std::string toString(const size_t indent) const override {
			std::string res = space(indent) + openBrace.value + "\n";

			for(const StatementNode* s : statements)
				res += s->toString(indent + 1) + "\n";

			res += space(indent) + closeBrace.value;

			return res;
		}
	};

	struct ReturnStatement : public StatementNode {
		Token returnToken;
		const ExpressionNode* expr;
		Token semicolon;

		inline ReturnStatement(const Token& returnToken, const ExpressionNode* expr, const Token& semicolon):
			StatementNode(Type::RETURN_STATEMENT),
			returnToken(returnToken), expr(expr), semicolon(semicolon) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			console << RBRANCH << "    IfStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			console << subIndent << VBRANCH << returnToken.value << "    ReturnKeyword " << returnToken.span << "\n";
			expr->print(console, subIndent, false);
			console << subIndent << LBRANCH << semicolon.value << "    semicolon " << semicolon.span << "\n";
		}

		inline virtual Span span() const override { return Span(returnToken.span, semicolon.span); }

		inline virtual std::string toString(const size_t indent) const override { return space(indent) + returnToken.value + " " + expr->toString(0) + semicolon.value; }
	};

	struct IfStatement : public StatementNode {
		Token ifToken;
		Token openParen;
		const ExpressionNode* condition;
		Token closeParen;
		const StatementNode* body;

		inline IfStatement(const Token& ifToken, const Token& openParen, const ExpressionNode* condition, const Token& closeParen, const StatementNode* body):
			StatementNode(Type::IF_STATEMENT),
			ifToken(ifToken), openParen(openParen), condition(condition), closeParen(closeParen), body(body) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			console << RBRANCH << "    IfStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			console << subIndent << VBRANCH << ifToken.value << "    IfKeyword " << ifToken.span << "\n";
			console << subIndent << VBRANCH << openParen.value << "    OpenParen " << openParen.span << "\n";
			condition->print(console, subIndent, false);
			console << subIndent << VBRANCH << closeParen.value << "    CloseParen " << closeParen.span << "\n";
			body->print(console, subIndent, true);
		}

		inline virtual Span span() const override { return Span(ifToken.span, body->span()); }

		inline virtual std::string toString(const size_t indent) const override { return space(indent) + ifToken.value + openParen.value + condition->toString(0) + closeParen.value + "\n" + body->toString(indent); }
	};

	struct WhileStatement : public StatementNode {
		Token whileToken;
		Token openParen;
		const ExpressionNode* condition;
		Token closeParen;
		const StatementNode* body;

		inline WhileStatement(const Token& whileToken, const Token& openParen, const ExpressionNode* condition, const Token& closeParen, const StatementNode* body):
			StatementNode(Type::WHILE_STATEMENT),
			whileToken(whileToken), openParen(openParen), condition(condition), closeParen(closeParen), body(body) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			console << RBRANCH << "    WhileStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			console << subIndent << VBRANCH << whileToken.value << "    WhileKeyword " << whileToken.span << "\n";
			console << subIndent << VBRANCH << openParen.value << "    OpenParen " << openParen.span << "\n";
			condition->print(console, subIndent, false);
			console << subIndent << VBRANCH << closeParen.value << "    CloseParen " << closeParen.span << "\n";
			body->print(console, subIndent, true);
		}

		inline virtual Span span() const override { return Span(whileToken.span, body->span()); }

		inline virtual std::string toString(const size_t indent) const override { return space(indent) + whileToken.value + openParen.value + condition->toString(0) + closeParen.value + "\n" + body->toString(indent); }
	};

	struct ArgumentsNode {
		struct Argument { Token type, name; };
		std::vector<Argument> args;
		std::vector<Token> commas;

		inline ArgumentsNode(const std::vector<Argument>& args, const std::vector<Token>& commas):
			args(args), commas(commas) {}

		inline void print(std::ostream& console, const std::string& indent, const bool isLast) const {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			console << RBRANCH << "    ArgumentList " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			if(args.size() == 0) {
				console << subIndent << LBRANCH << "<empty>\n";
				return;
			}

			for(size_t i = 0; i < commas.size(); i++) {
				console << subIndent << VBRANCH << args[i].type.value << "    Typename " << args[i].type.span << "\n";
				console << subIndent << VBRANCH << args[i].name.value << "    Identifier " << args[i].name.span << "\n";
				console << subIndent << VBRANCH << commas[i].value << "    Comma " << commas[i].span << "\n";
			}

			console << subIndent << VBRANCH << args.back().type.value << "    Typename " << args.back().type.span << "\n";
			console << subIndent << LBRANCH << args.back().name.value << "    Identifier " << args.back().name.span << "\n";
		}

		inline Span span() const { return args.size()>0 ? Span(args.front().type.span, args.back().name.span) : Span(static_cast<size_t>(-1), static_cast<size_t>(-1)); }

		inline std::string toString(const size_t indent) const {
			std::string res;

			for(size_t i = 0; i < commas.size(); i++)
				res += args[i].type.value + " " + args[i].name.value + commas[i].value + " ";

			if(args.size() > 0)
				res += args.back().type.value + " " + args.back().name.value;

			return res;
		}
	};

	struct FunctionDeclarationStatement : public StatementNode {
		Token typeName;
		Token functionName;
		Token openParen;
		const ArgumentsNode* args;
		Token closeParen;
		const StatementNode* body;

		inline FunctionDeclarationStatement(const Token& typeName, const Token& functionName, const Token& openParen, const ArgumentsNode* args, const Token& closeParen, const StatementNode* body):
			StatementNode(Type::FUNCTION_DECLARATION),
			typeName(typeName), functionName(functionName), openParen(openParen), args(args), closeParen(closeParen), body(body) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			console << RBRANCH << "    FunctionDeclarationStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			console << subIndent << VBRANCH << typeName.value << "    Typename " << typeName.span << "\n";
			console << subIndent << VBRANCH << functionName.value << "    Identifier " << functionName.span << "\n";
			console << subIndent << VBRANCH << openParen.value << "    OpenParen " << openParen.span << "\n";
			args->print(console, subIndent, false);
			console << subIndent << VBRANCH << closeParen.value << "    CloseParen " << closeParen.span << "\n";
			body->print(console, subIndent, true);
		}

		inline virtual Span span() const override { return Span(typeName.span, body->span()); }

		inline virtual std::string toString(const size_t indent) const override { return space(indent) + typeName.value + " " + functionName.value + openParen.value + args->toString(0) + closeParen.value + "\n" + body->toString(indent) + "\n"; }
	};

	// Program:
	struct Program : public Node {
		std::vector<const StatementNode*> statements;

		inline Program(const std::vector<const StatementNode*>& statements):
			Node(Node::BaseType::PROGRAM),
			statements(statements) {}

		inline virtual void print(std::ostream& console, const std::string& indent, const bool isLast) const override {
			console << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			console << RBRANCH << "    Program " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			for(const StatementNode* statement : statements)
				statement->print(console, subIndent, statement == statements.back());
		}

		inline virtual Span span() const override { return statements.size()>0 ? Span(statements.front()->span(), statements.back()->span()) : Span(static_cast<size_t>(-1), static_cast<size_t>(-1)); }

		inline virtual std::string toString(const size_t indent) const override {
			std::string res;

			for(const StatementNode* s : statements)
				res += s->toString(indent) + "\n";

			return res;
		}
	};
};

