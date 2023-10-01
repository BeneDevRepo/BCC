#pragma once


#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

#include "Tokens.hpp"


inline std::string space(const size_t indent) { std::string res; for(size_t i = 0; i < indent; i++) res += "  "; return res; }

class Parser {
private:
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

public:
	struct Node {
		inline virtual ~Node() {}
		inline virtual void print(const std::string& indent = "", const bool isLast = true) const = 0;
		inline virtual Span span() const = 0;
		inline virtual std::string toString(const size_t indent = 0) const = 0;
	};


	// expressions:
	struct ExpressionNode : public Node { };

	struct UnaryExpressionNode : public ExpressionNode {
		Token op; // operation
		ExpressionNode *a;

		inline UnaryExpressionNode(ExpressionNode* a, const Token& op): op(op), a(a) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << op.value;
			std::cout << "    UnaryExpression " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, true);
		}

		inline virtual Span span() const { return Span(a->span(), op.span); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + op.value + a->toString(0); }
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

		inline virtual Span span() const { return Span(a->span(), b->span()); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + a->toString(0) + " " + op.value + " " + b->toString(0); }
	};

	struct IdentifierNode : public ExpressionNode {
		Token name;

		inline IdentifierNode(const Token& name): name(name) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << "<" << name.value << ">";
			std::cout << "    Identifier " << span() << "\n";
		}

		inline virtual Span span() const { return name.span; }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + name.value; }
	};

	struct LiteralNode : public ExpressionNode {
		Token value;

		inline LiteralNode(const Token& value): value(value) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH) << value.value;
			std::cout << "    Literal " << span() << "\n";
		}

		inline virtual Span span() const { return value.span; }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + value.value; }
	};


	// Statements:
	struct StatementNode : public Node { };

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

		inline virtual Span span() const { return Span(typeName.span, semicolon.span); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + typeName.value + " " + varName->toString(0) + " " + equals.value + " " + expr->toString(0) + semicolon.value; }
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

		inline virtual Span span() const { return Span(openBrace.span, closeBrace.span); }

		inline virtual std::string toString(const size_t indent) const {
			std::string res = space(indent) + openBrace.value + "\n";

			for(const StatementNode* s : statements)
				res += s->toString(indent + 1) + "\n";

			res += space(indent) + closeBrace.value;

			return res;
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

		inline virtual Span span() const { return Span(ifToken.span, body->span()); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + ifToken.value + openParen.value + condition->toString(0) + closeParen.value + "\n" + body->toString(indent); }
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

		inline virtual Span span() const { return Span(whileToken.span, body->span()); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + whileToken.value + openParen.value + condition->toString(0) + closeParen.value + "\n" + body->toString(indent); }
	};

	struct ArgumentsNode : public ExpressionNode {
		struct Argument { Token type, name; };
		std::vector<Argument> args;
		std::vector<Token> commas;

		inline ArgumentsNode(const std::vector<Argument>& args, const std::vector<Token>& commas): args(args), commas(commas) {}

		inline virtual void print(const std::string& indent, const bool isLast) const { // TODO
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

		inline virtual Span span() const { return args.size()>0 ? Span(args.front().type.span, args.back().name.span) : Span(static_cast<size_t>(-1), static_cast<size_t>(-1)); }

		inline virtual std::string toString(const size_t indent) const {
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
		Token functionName; // TODO: fix
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

		inline virtual Span span() const { return Span(typeName.span, body->span()); }

		inline virtual std::string toString(const size_t indent) const { return space(indent) + typeName.value + " " + functionName.value + openParen.value + args->toString(0) + closeParen.value + "\n" + body->toString(indent) + "\n"; }
	};

	// Program:
	struct Program : public Node {
		std::vector<StatementNode*> statements;

		inline Program(const std::vector<StatementNode*>& statements): statements(statements) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"

			std::cout << RBRANCH << "    Program " << span() << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			// std::cout << subIndent << VBRANCH << openBrace.value << "    OpenBrace " << openBrace.span << "\n";

			for(StatementNode* statement : statements)
				statement->print(subIndent, statement == statements.back());

			// std::cout << subIndent << LBRANCH << closeBrace.value << "    CloseBrace " << closeBrace.span << "\n";
		}

		inline virtual Span span() const { return statements.size()>0 ? Span(statements.front()->span(), statements.back()->span()) : Span(static_cast<size_t>(-1), static_cast<size_t>(-1)); }

		inline virtual std::string toString(const size_t indent) const {
			std::string res;

			for(const StatementNode* s : statements)
				res += s->toString(indent) + "\n";

			return res;
		}
	};

private:
	TokenProvider& tokenProvider;
	// std::vector<Token> tokens;
	// size_t currentToken;

public:
	inline Parser(TokenProvider& tokenProvider): tokenProvider(tokenProvider) {} // parser does not own tokenProvider, it only uses it
	inline const Token peekToken() const { return tokenProvider.peek(); }
	inline const Token getToken() { return tokenProvider.consume(); }
	// inline const Token peekToken() const {
	// 	Token t = tokenProvider->peek();
	// 	std::cout << "Peek(): " << t.value << " " << t.span << "\n";
	// 	return t;
	// }
	// inline const Token getToken() {
	// 	Token t = tokenProvider->consume();
	// 	std::cout << "getToken(): " << t.value << " " << t.span << "\n";
	// 	return t;
	// }

	// ###########
	// # Program #
	// ###########
	inline Program* program() {
		std::vector<StatementNode*> statements;

		while(StatementNode* stm = statement())
			statements.push_back(stm);

		return new Program(statements);
	}
	
	// ##############
	// # STATEMENTS #
	// ##############
	inline StatementNode* statement() {
		if(BlockStatement* block = blockStatement())
			return block;

		if(VariableDeclarationStatement* varDecl = variableDeclaration())
			return varDecl;

		if(IfStatement* ifStmt = ifStatement())
			return ifStmt;

		if(WhileStatement* whileStmt = whileStatement())
			return whileStmt;

		if(FunctionDeclarationStatement* function = functionDeclaration())
			return function;

		return nullptr;
	}

	inline BlockStatement* blockStatement() {
		if(peekToken().type != Token::Type::BRACE_OPEN)
			return nullptr;
		
		const Token& openBrace = getToken(); // consume '{'
		
		std::vector<StatementNode*> statements;

		while(StatementNode* stm = statement())
			statements.push_back(stm);
		
		if(peekToken().type != Token::Type::BRACE_CLOSE)
			throw std::runtime_error("Block did not end with '}'");

		const Token& closeBrace = getToken(); // consume '}'

		return new BlockStatement(openBrace, statements, closeBrace);
	}

	inline VariableDeclarationStatement* variableDeclaration() {
		static constexpr auto isTypename = [](const Token& token) { const Token::Type type = token.type; return type == Token::Type::BOOL || type == Token::Type::INT || type == Token::Type::FLOAT || type == Token::Type::STRING; };

		tokenProvider.pushState();
		// const size_t currentTokenBefore = currentToken;

		if(!isTypename(peekToken())) {
			tokenProvider.popState();
			return nullptr;
		}
		
		const Token& type = getToken(); // consume typename

		IdentifierNode* name = variable();

		if(!name) {
			tokenProvider.popState();
			return nullptr;
		}

		if(peekToken().type == Token::Type::SEMICOLON) {
			const Token& semicolon = getToken(); // consume ';'
			tokenProvider.yeetState();
			return new VariableDeclarationStatement(type, name, semicolon); // pure declaration
		}
		

		if(peekToken().type != Token::Type::EQUAL) {
			throw std::runtime_error("Failed to parse Variable Declaration!");
			tokenProvider.popState();
			return nullptr;
		}

		const Token& equals = getToken(); // consume '='

		ExpressionNode* expr = expression();

		if(!expr) {
			throw std::runtime_error("Failed to parse Variable Declaration!");
			tokenProvider.popState();
			return nullptr;
		}

		if(peekToken().type != Token::Type::SEMICOLON) {
			throw std::runtime_error("Failed to parse Variable Declaration!");
			tokenProvider.popState();
			return nullptr;
		}

		const Token& semicolon = getToken(); // consume ';'

		tokenProvider.yeetState();
		return new VariableDeclarationStatement(type, name, equals, expr, semicolon);
	}

	inline IfStatement* ifStatement() {
		tokenProvider.pushState();

		if(peekToken().type != Token::Type::IF) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& ifToken = getToken(); // consume 'if'


		if(peekToken().type != Token::Type::PAREN_OPEN) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& openParen = getToken(); // consume '('


		ExpressionNode* condition = expression();
		if(!condition) {
			tokenProvider.popState();
			return nullptr;
		}


		if(peekToken().type != Token::Type::PAREN_CLOSE) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& closeParen = getToken(); // consume ')'


		StatementNode* body = statement();
		if(!body) {
			tokenProvider.popState();
			return nullptr;
		}

		tokenProvider.yeetState();
		return new IfStatement(ifToken, openParen, condition, closeParen, body);
	}

	inline WhileStatement* whileStatement() {
		tokenProvider.pushState();

		if(peekToken().type != Token::Type::WHILE) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& whileToken = getToken(); // consume 'while'


		if(peekToken().type != Token::Type::PAREN_OPEN) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& openParen = getToken(); // consume '('


		ExpressionNode* condition = expression();
		if(!condition) {
			tokenProvider.popState();
			return nullptr;
		}


		if(peekToken().type != Token::Type::PAREN_CLOSE) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& closeParen = getToken(); // consume ')'


		StatementNode* body = statement();
		if(!body) {
			tokenProvider.popState();
			return nullptr;
		}

		tokenProvider.yeetState();
		return new WhileStatement(whileToken, openParen, condition, closeParen, body);
	}

	inline ArgumentsNode* argumentList() {
		static constexpr auto isTypename = [](const Token& token) { const Token::Type type = token.type; return type == Token::Type::BOOL || type == Token::Type::INT || type == Token::Type::FLOAT || type == Token::Type::STRING; };

		std::vector<ArgumentsNode::Argument> args;
		std::vector<Token> commas;

		tokenProvider.pushState();

		if(!isTypename(peekToken())) {
			tokenProvider.yeetState();
			return new ArgumentsNode(args, commas);
		}

		for(;;) {
			ArgumentsNode::Argument arg;
			
			if(!isTypename(peekToken())) {
				tokenProvider.popState();
				return nullptr;
			}
			arg.type = getToken(); // consume argument type

			if(peekToken().type != Token::Type::IDENTIFIER) {
				tokenProvider.popState();
				return nullptr;
			}
			arg.name = getToken(); // consume argument name

			args.push_back(arg);

			if(peekToken().type != Token::Type::COMMA)
				break;
			commas.push_back(getToken()); // consume ','
		}

		tokenProvider.yeetState();
		return new ArgumentsNode(args, commas);
	}

	inline FunctionDeclarationStatement* functionDeclaration() {
		static constexpr auto isTypename = [](const Token& token) { const Token::Type type = token.type; return type == Token::Type::VOID || type == Token::Type::BOOL || type == Token::Type::INT || type == Token::Type::FLOAT || type == Token::Type::STRING; };

		tokenProvider.pushState();

		if(!isTypename(peekToken())) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& typeName = getToken(); // consume typename


		if(peekToken().type != Token::Type::IDENTIFIER) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& name = getToken(); // consume functionName


		if(peekToken().type != Token::Type::PAREN_OPEN) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& openParen = getToken(); // consume '('


		ArgumentsNode* args = argumentList();
		if(!args) {
			tokenProvider.popState();
			return nullptr;
		}


		if(peekToken().type != Token::Type::PAREN_CLOSE) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& closeParen = getToken(); // consume ')'


		StatementNode* body = statement();
		if(!body) {
			tokenProvider.popState();
			return nullptr;
		}


		tokenProvider.yeetState();
		return new FunctionDeclarationStatement(typeName, name, openParen, args, closeParen, body);
	}


	// ###############
	// # EXPRESSIONS #
	// ###############
	inline ExpressionNode* expression() {
		return additiveExpression();
	}
	
	inline ExpressionNode* additiveExpression() {
		constexpr static auto isSumType = [](const Token::Type type) { return type == Token::Type::PLUS || type == Token::Type::MINUS; };

		ExpressionNode* a = multiplicativeExpression();
		
		while(isSumType(peekToken().type)) {
			const Token& op = getToken(); // consume operation token
			ExpressionNode* b = multiplicativeExpression();
			a = new BinaryExpressionNode(a, b, op);
		}

		return a;
	}

	inline ExpressionNode* multiplicativeExpression() {
		constexpr static auto isMulType = [](const Token::Type& type) { return type == Token::Type::MUL || type == Token::Type::DIV; };

		ExpressionNode* a = primaryExpression();

		while(isMulType(peekToken().type)) {
			const Token& op = getToken(); // consume operation token
			ExpressionNode* b = primaryExpression();
			a = new BinaryExpressionNode(a, b, op);
		}

		return a;
	}

	inline ExpressionNode* primaryExpression() {
		// group:
		if(peekToken().type == Token::Type::PAREN_OPEN) {
			getToken(); // consume '('
			ExpressionNode *expr = expression();
			if(peekToken().type != Token::Type::PAREN_CLOSE)
				throw std::runtime_error("Missing closing Parenthesis at the end of primary expression");
			getToken(); // consume ')'
			return expr;
		}

		// literal:
		if(ExpressionNode* n = literal())
			return n;
		
		// variable name:
		if(ExpressionNode* v = variable())
			return v;
		
		// negation:
		if(peekToken().type == Token::Type::MINUS) {
			const Token& op = getToken(); // consume '-'
			return new UnaryExpressionNode(primaryExpression(), op);
		}

		return nullptr;
	}

	inline IdentifierNode* variable() {
		if(peekToken().type == Token::Type::IDENTIFIER)
			return new IdentifierNode(getToken());
			
		return nullptr;
	}


	inline ExpressionNode* literal() {
		constexpr static auto isLiteralType = [](const Token::Type type) {
			switch(type) {
				case Token::Type::BOOL_LITERAL:
				case Token::Type::INT_LITERAL:
				case Token::Type::FLOAT_LITERAL:
				case Token::Type::STRING_LITERAL:
				return true;
			}
			return false;
		};

		if(isLiteralType(peekToken().type))
			return new LiteralNode(getToken());
		
		return nullptr;
	}

	inline Program* parse() {
		Program* tree = program();

		return tree;
	}
};
