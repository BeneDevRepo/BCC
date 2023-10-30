#pragma once


#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

#include "ScopedSymbolTable.hpp"
#include "Tokens.hpp"
#include "AST.hpp"


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
	
		enum class Type : uint8_t {
			PROGRAM,

			LITERAL_EXPRESSION, VARIABLE_EXPRESSION, UNARY_EXPRESSION, BINARY_EXPRESSION, CALL_EXPRESSION, GROUP_EXPRESSION,

			EXPRESSION_STATEMENT, BLOCK_STATEMENT, RETURN_STATEMENT,
			IF_STATEMENT, WHILE_STATEMENT, FUNCTION_DECLARATION, VARIABLE_DECLARATION, VARIABLE_ASSIGNMENT,
		};

	private:
		BaseType baseType_;
		Type type_;

	public:
		inline Node(const BaseType baseType, const Type type): baseType_(baseType), type_(type) {}
		inline virtual ~Node() {}

	public:
		inline BaseType baseType() const { return baseType_; }
		inline Type type() const { return type_; }
	
	public:
		inline virtual void print(const std::string& indent = "", const bool isLast = true) const = 0;
		inline virtual Span span() const = 0;
		inline virtual std::string toString(const size_t indent = 0) const = 0;
	};


	struct ExpressionNode : public Node {
		inline ExpressionNode(const Type type): Node(BaseType::EXPRESSION, type) {}
		inline virtual const AST::ExpressionNode* ast(ScopedSymbolTable* scope) const = 0;
	};

	struct StatementNode : public Node {
		inline StatementNode(const Type type): Node(BaseType::STATEMENT, type) {}
		inline virtual const AST::StatementNode* ast(ScopedSymbolTable* scope) const = 0;
	};


	// expressions:
	struct FunctionCallExpressionNode : public ExpressionNode {
		Token name; // function name
		Token openParen;
		std::vector<const ExpressionNode*> args; // function call arguments
		std::vector<Token> commas; // commas between function call arguments
		Token closeParen;

		inline FunctionCallExpressionNode(const Token& name, const Token& openParen, const std::vector<const ExpressionNode*>& args, const std::vector<Token>& commas, const Token& closeParen):
			ExpressionNode(Node::Type::CALL_EXPRESSION),
			name(name), openParen(openParen), args(args), commas(commas), closeParen(closeParen) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    FunctionCall " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << name.value << "    Identifier " << name.span << "\n";
			std::cout << subIndent << VBRANCH << openParen.value << "    OpenParen " << openParen.span << "\n";
			for(const ExpressionNode* arg : args)
				arg->print(subIndent, false);
			std::cout << subIndent << LBRANCH << closeParen.value << "    CloseParen " << closeParen.span << "\n";
		}

		inline virtual Span span() const { return Span(name.span, closeParen.span); }

		inline virtual std::string toString(const size_t indent) const {
			std::string res = space(indent) + name.value + openParen.value;
			for(size_t i = 0; i < commas.size(); i++)
				res += args[i]->toString(0) + commas[i].value + " ";
			if(args.size() > 0)
				res += args.back()->toString(0);
			res += closeParen.value;
			return res;
		}

		inline virtual const AST::ExpressionNode* ast(ScopedSymbolTable* scope) const {
			std::vector<const AST::ExpressionNode*> astArgs;
			for(const ExpressionNode* arg : args)
				astArgs.push_back(arg->ast(scope));
			return new AST::FunctionCallExpressionNode(scope, name.value, astArgs);
		}
	};

	struct GroupExpressionNode : public ExpressionNode {
		Token openParen;
		const ExpressionNode *a;
		Token closeParen;

		inline GroupExpressionNode(const Token& openParen, const ExpressionNode* a, const Token& closeParen):
			ExpressionNode(Node::Type::GROUP_EXPRESSION),
			openParen(openParen), a(a), closeParen(closeParen) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH;
			std::cout << "    GroupExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << openParen.value << "\n";
			a->print(subIndent, false);
			std::cout << subIndent << LBRANCH << closeParen.value << "\n";
		}

		inline virtual Span span() const { return Span(openParen.span, closeParen.span); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + openParen.value + a->toString(0) + closeParen.value; }

		inline virtual const AST::ExpressionNode* ast(ScopedSymbolTable* scope) const {
			return a->ast(scope);
		}
	};

	struct UnaryExpressionNode : public ExpressionNode {
		Token op; // operation
		const ExpressionNode *a;

		inline UnaryExpressionNode(const Token& op, const ExpressionNode* a):
			ExpressionNode(Node::Type::UNARY_EXPRESSION),
			op(op), a(a) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << op.value;
			std::cout << "    UnaryExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, true);
		}

		inline virtual Span span() const { return Span(a->span(), op.span); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + op.value + a->toString(0); }

		inline virtual const AST::ExpressionNode* ast(ScopedSymbolTable* scope) const {
			return new AST::UnaryExpressionNode(scope, op.value, a->ast(scope));
		}
	};

	struct BinaryExpressionNode : public ExpressionNode {
		const ExpressionNode *a;
		Token op; // operation
		const ExpressionNode *b;

		inline BinaryExpressionNode(const ExpressionNode* a, const Token& op, const ExpressionNode* b):
			ExpressionNode(Node::Type::BINARY_EXPRESSION),
			a(a), op(op), b(b) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << op.value;
			std::cout << "    BinaryExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, false);
			b->print(subIndent, true);
		}

		inline virtual Span span() const { return Span(a->span(), b->span()); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + a->toString(0) + " " + op.value + " " + b->toString(0); }

		inline virtual const AST::ExpressionNode* ast(ScopedSymbolTable* scope) const {
			return new AST::BinaryExpressionNode(scope, a->ast(scope), op.value, b->ast(scope));
		}
	};

	struct IdentifierNode : public ExpressionNode {
		Token name;

		inline IdentifierNode(const Token& name):
			ExpressionNode(Node::Type::VARIABLE_EXPRESSION),
			name(name) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << "<" << name.value << ">";
			std::cout << "    Identifier " << span() << "\n";
		}

		inline virtual Span span() const { return name.span; }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + name.value; }

		inline virtual const AST::ExpressionNode* ast(ScopedSymbolTable* scope) const {
			return new AST::IdentifierNode(scope, name.value);
		}
	};

	struct LiteralNode : public ExpressionNode {
		Token value;

		inline LiteralNode(const Token& value):
			ExpressionNode(Node::Type::LITERAL_EXPRESSION),
			value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << value.value;
			std::cout << "    Literal " << span() << "\n";
		}

		inline virtual Span span() const { return value.span; }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + value.value; }

		inline virtual const AST::LiteralNode* ast(ScopedSymbolTable* scope) const {
			switch(value.type) {
				case Token::Type::INT_LITERAL:
					return new AST::IntLiteralNode(scope, std::stoi(value.value));
			}
			throw std::runtime_error("Error generating literal AST Node: Token is not a known literal type");
		}
	};


	// Statements:
	struct VariableDeclarationStatement : public StatementNode {
		Token typeName;
		Token varName;
		Token equals;
		const ExpressionNode* expr;
		Token semicolon;

		inline VariableDeclarationStatement(const Token& typeName, const Token& varName, const Token& equals, const ExpressionNode* expr, const Token& semicolon):
			StatementNode(Node::Type::VARIABLE_DECLARATION),
			typeName(typeName), varName(varName), equals(equals), expr(expr), semicolon(semicolon) {}
		inline VariableDeclarationStatement(const Token& typeName, const Token& varName, const Token& semicolon):
			VariableDeclarationStatement(typeName, varName, equals, nullptr, semicolon) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << RBRANCH << "    Declaration " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << typeName.value << "    Typename " << typeName.span << "\n";

			std::cout << subIndent << VBRANCH << varName.value << "    Identifier " << varName.span << "\n";

			if(expr) {
				std::cout << subIndent << VBRANCH << equals.value << "    Operator " << equals.span << "\n";
				expr->print(subIndent, false);
			}

			std::cout << subIndent << LBRANCH << semicolon.value << "    Semicolon " << span() << "\n";
		}

		inline virtual Span span() const { return Span(typeName.span, semicolon.span); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + typeName.value + " " + varName.value + (expr ? (" " + equals.value + " " + expr->toString(0)) : "") + semicolon.value; }

		inline virtual const AST::StatementNode* ast(ScopedSymbolTable* scope) const {
			if(!expr)
				return new AST::VariableDeclarationStatement(scope, typeName.value, varName.value);

			const AST::VariableAssignmentStatement* assignment = new AST::VariableAssignmentStatement(scope, varName.value, expr->ast(scope));
			return new AST::VariableDeclarationStatement(scope, typeName.value, varName.value, assignment);
		}
	};

	struct ExpressionStatement : public StatementNode {
		const ExpressionNode* expr;
		Token semicolon;

		inline ExpressionStatement(const ExpressionNode* expr, const Token& semicolon):
			StatementNode(Node::Type::EXPRESSION_STATEMENT),
			expr(expr), semicolon(semicolon) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    ExpressionStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			expr->print(subIndent, false);

			std::cout << subIndent << LBRANCH << semicolon.value << "    Semicolon " << semicolon.span << "\n";
		}

		inline virtual Span span() const { return Span(expr->span(), semicolon.span); }

		inline virtual std::string toString(const size_t indent) const {
			return expr->toString(indent) + semicolon.value;
		}

		inline virtual const AST::StatementNode* ast(ScopedSymbolTable* scope) const {
			return new AST::ExpressionStatement(scope, expr->ast(scope));
		}
	};

	struct BlockStatement : public StatementNode {
		Token openBrace;
		std::vector<const StatementNode*> statements;
		Token closeBrace;
		mutable bool createScope;

		inline BlockStatement(const Token& openBrace, const std::vector<const StatementNode*>& statements, const Token& closeBrace):
			StatementNode(Node::Type::BLOCK_STATEMENT),
			openBrace(openBrace), statements(statements), closeBrace(closeBrace), createScope(true) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    Block " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << openBrace.value << "    OpenBrace " << openBrace.span << "\n";

			for(const StatementNode* statement : statements)
				statement->print(subIndent, false);

			std::cout << subIndent << LBRANCH << closeBrace.value << "    CloseBrace " << closeBrace.span << "\n";
		}

		inline virtual Span span() const { return Span(openBrace.span, closeBrace.span); }

		inline virtual std::string toString(const size_t indent) const {
			std::string res = space(indent) + openBrace.value + "\n";

			for(const StatementNode* s : statements)
				res += s->toString(indent + 1) + "\n";

			res += space(indent) + closeBrace.value;

			return res;
		}

		inline virtual const AST::StatementNode* ast(ScopedSymbolTable* scope) const {
			ScopedSymbolTable* localScope = createScope ? new ScopedSymbolTable("Local Block Scope", scope) : scope;

			std::vector<const AST::StatementNode*> astStatements;

			for(const StatementNode* statement : statements)
				astStatements.push_back(statement->ast(localScope));

			return new AST::StatementList(localScope, astStatements);
		}
	};

	struct ReturnStatement : public StatementNode {
		Token returnToken;
		const ExpressionNode* expr;
		Token semicolon;

		inline ReturnStatement(const Token& returnToken, const ExpressionNode* expr, const Token& semicolon):
			StatementNode(Node::Type::RETURN_STATEMENT),
			returnToken(returnToken), expr(expr), semicolon(semicolon) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    IfStatement " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			std::cout << subIndent << VBRANCH << returnToken.value << "    ReturnKeyword " << returnToken.span << "\n";
			expr->print(subIndent, false);
			std::cout << subIndent << LBRANCH << semicolon.value << "    semicolon " << semicolon.span << "\n";
		}

		inline virtual Span span() const { return Span(returnToken.span, semicolon.span); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + returnToken.value + " " + expr->toString(0) + semicolon.value; }

		inline virtual const AST::StatementNode* ast(ScopedSymbolTable* scope) const {
			return new AST::ReturnStatement(scope, expr->ast(scope));
		}
	};

	struct IfStatement : public StatementNode {
		Token ifToken;
		Token openParen;
		const ExpressionNode* condition;
		Token closeParen;
		const StatementNode* body;

		inline IfStatement(const Token& ifToken, const Token& openParen, const ExpressionNode* condition, const Token& closeParen, const StatementNode* body):
			StatementNode(Node::Type::IF_STATEMENT),
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

		inline virtual Span span() const { return Span(ifToken.span, body->span()); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + ifToken.value + openParen.value + condition->toString(0) + closeParen.value + "\n" + body->toString(indent); }

		inline virtual const AST::StatementNode* ast(ScopedSymbolTable* scope) const {
			return new AST::IfStatement(scope, condition->ast(scope), body->ast(scope));
		}
	};

	struct WhileStatement : public StatementNode {
		Token whileToken;
		Token openParen;
		const ExpressionNode* condition;
		Token closeParen;
		const StatementNode* body;

		inline WhileStatement(const Token& whileToken, const Token& openParen, const ExpressionNode* condition, const Token& closeParen, const StatementNode* body):
			StatementNode(Node::Type::WHILE_STATEMENT),
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

		inline virtual Span span() const { return Span(whileToken.span, body->span()); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + whileToken.value + openParen.value + condition->toString(0) + closeParen.value + "\n" + body->toString(indent); }

		inline virtual const AST::StatementNode* ast(ScopedSymbolTable* scope) const {
			return new AST::WhileStatement(scope, condition->ast(scope), body->ast(scope));
		}
	};

	struct ArgumentsNode {
		struct Argument { Token type, name; };
		std::vector<Argument> args;
		std::vector<Token> commas;

		inline ArgumentsNode(const std::vector<Argument>& args, const std::vector<Token>& commas):
			args(args), commas(commas) {}

		inline void print(const std::string& indent, const bool isLast) const {
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
			StatementNode(Node::Type::FUNCTION_DECLARATION),
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

		inline virtual Span span() const { return Span(typeName.span, body->span()); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + typeName.value + " " + functionName.value + openParen.value + args->toString(0) + closeParen.value + "\n" + body->toString(indent) + "\n"; }

		inline virtual const AST::StatementNode* ast(ScopedSymbolTable* scope) const {
			ScopedSymbolTable* localScope = new ScopedSymbolTable("Local Function Scope", scope);

			std::vector<AST::FunctionDeclarationStatement::Argument> astArgs;

			for(const auto& arg : args->args)
				astArgs.push_back({ arg.type.value, arg.name.value });

			return new AST::FunctionDeclarationStatement(localScope, typeName.value, functionName.value, astArgs, body->ast(localScope));
		}
	};

	// Program:
	struct Program : public Node {
		std::vector<const StatementNode*> statements;

		inline Program(const std::vector<const StatementNode*>& statements):
			Node(Node::BaseType::PROGRAM, Node::Type::PROGRAM),
			statements(statements) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    Program " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "

			for(const StatementNode* statement : statements)
				statement->print(subIndent, statement == statements.back());
		}

		inline virtual Span span() const { return statements.size()>0 ? Span(statements.front()->span(), statements.back()->span()) : Span(static_cast<size_t>(-1), static_cast<size_t>(-1)); }

		inline virtual std::string toString(const size_t indent) const {
			std::string res;

			for(const StatementNode* s : statements)
				res += s->toString(indent) + "\n";

			return res;
		}

		inline const AST::StatementList* ast(ScopedSymbolTable* scope) const {
			std::vector<const AST::StatementNode*> astStatements;

			for(const StatementNode* statement : statements)
				astStatements.push_back(statement->ast(scope));
			
			return new AST::StatementList(scope, astStatements);
		}
	};
};

