#pragma once


#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

#include "Tokens.hpp"
#include "ParseTree.hpp"



class Parser {
private:
	TokenProvider& tokenProvider;

public:
	inline Parser(TokenProvider& tokenProvider): tokenProvider(tokenProvider) {} // parser does not own tokenProvider, it only uses it
	inline const Token peekToken() const { return tokenProvider.peek(); }
	inline const Token getToken() { return tokenProvider.consume(); }


	// ###########
	// # Program #
	// ###########
	inline const ParseTree::Program* program() {
		std::vector<const ParseTree::StatementNode*> statements;

		while(const ParseTree::StatementNode* stm = statement())
			statements.push_back(stm);
		
		if(peekToken().type != Token::Type::END)
			throw std::runtime_error("Unable to parse program till EOF token");

		return new ParseTree::Program(statements);
	}
	
	// ##############
	// # STATEMENTS #
	// ##############
	inline const ParseTree::StatementNode* statement() {
		if(const ParseTree::BlockStatement* block = blockStatement())
			return block;

		if(const ParseTree::ReturnStatement* ret = returnStatement())
			return ret;

		if(const ParseTree::VariableDeclarationStatement* varDecl = variableDeclaration())
			return varDecl;

		if(const ParseTree::IfStatement* ifStmt = ifStatement())
			return ifStmt;

		if(const ParseTree::WhileStatement* whileStmt = whileStatement())
			return whileStmt;

		if(const ParseTree::FunctionDeclarationStatement* function = functionDeclaration())
			return function;

		if(const ParseTree::ExpressionStatement* expStmt = expressionStatement())
			return expStmt;

		return nullptr;
	}

	inline const ParseTree::ExpressionStatement* expressionStatement() {
		tokenProvider.pushState();
		
		const ParseTree::ExpressionNode* expr = expression();

		if(!expr) {
			tokenProvider.popState();
			return nullptr;
		}
		
		if(peekToken().type != Token::Type::SEMICOLON) {
			tokenProvider.popState();
			return nullptr;
		}

		const Token& semicolon = getToken(); // consume ';'

		tokenProvider.yeetState();
		return new ParseTree::ExpressionStatement(expr, semicolon);
	}

	inline const ParseTree::BlockStatement* blockStatement() {
		if(peekToken().type != Token::Type::BRACE_OPEN)
			return nullptr;
		
		const Token& openBrace = getToken(); // consume '{'
		
		std::vector<const ParseTree::StatementNode*> statements;

		while(const ParseTree::StatementNode* stm = statement())
			statements.push_back(stm);
		
		if(peekToken().type != Token::Type::BRACE_CLOSE)
			throw std::runtime_error("Block did not end with '}'");

		const Token& closeBrace = getToken(); // consume '}'

		return new ParseTree::BlockStatement(openBrace, statements, closeBrace);
	}

	inline const ParseTree::ReturnStatement* returnStatement() {
		if(peekToken().type != Token::Type::RETURN)
			return nullptr;
		
		const Token& returnToken = getToken(); // consume 'return'
		
		const ParseTree::ExpressionNode* expr = expression();
		if(!expr || peekToken().type != Token::Type::SEMICOLON)
			throw std::runtime_error("Error parsing return value expression");
		
		const Token& semicolon = getToken();
		
		return new ParseTree::ReturnStatement(returnToken, expr, semicolon);
	}

	inline const ParseTree::VariableDeclarationStatement* variableDeclaration() {
		static constexpr auto isTypename = [](const Token& token) { const Token::Type type = token.type; return type == Token::Type::BOOL || type == Token::Type::INT || type == Token::Type::FLOAT || type == Token::Type::STRING; };

		tokenProvider.pushState();
		// const size_t currentTokenBefore = currentToken;

		if(!isTypename(peekToken())) {
			tokenProvider.popState();
			return nullptr;
		}
		
		const Token& type = getToken(); // consume typename

		// ParseTree::IdentifierNode* name = identifier();
		if(peekToken().type != Token::Type::IDENTIFIER) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& name = getToken();

		if(peekToken().type == Token::Type::SEMICOLON) {
			const Token& semicolon = getToken(); // consume ';'
			tokenProvider.yeetState();
			return new ParseTree::VariableDeclarationStatement(type, name, semicolon); // pure declaration
		}
		

		if(peekToken().type != Token::Type::EQUAL) {
			tokenProvider.popState();
			return nullptr;
		}

		const Token& equals = getToken(); // consume '='

		const ParseTree::ExpressionNode* expr = expression();

		if(!expr) {
			throw std::runtime_error("Failed to parse Variable Declaration!1");
			tokenProvider.popState();
			return nullptr;
		}

		if(peekToken().type != Token::Type::SEMICOLON) {
			throw std::runtime_error("Failed to parse Variable Declaration!2");
			tokenProvider.popState();
			return nullptr;
		}

		const Token& semicolon = getToken(); // consume ';'

		tokenProvider.yeetState();
		return new ParseTree::VariableDeclarationStatement(type, name, equals, expr, semicolon);
	}

	inline const ParseTree::IfStatement* ifStatement() {
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


		const ParseTree::ExpressionNode* condition = expression();
		if(!condition) {
			tokenProvider.popState();
			return nullptr;
		}


		if(peekToken().type != Token::Type::PAREN_CLOSE) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& closeParen = getToken(); // consume ')'


		const ParseTree::StatementNode* body = statement();
		if(!body) {
			tokenProvider.popState();
			return nullptr;
		}

		tokenProvider.yeetState();
		return new ParseTree::IfStatement(ifToken, openParen, condition, closeParen, body);
	}

	inline const ParseTree::WhileStatement* whileStatement() {
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


		const ParseTree::ExpressionNode* condition = expression();
		if(!condition) {
			tokenProvider.popState();
			return nullptr;
		}


		if(peekToken().type != Token::Type::PAREN_CLOSE) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& closeParen = getToken(); // consume ')'


		const ParseTree::StatementNode* body = statement();
		if(!body) {
			tokenProvider.popState();
			return nullptr;
		}

		tokenProvider.yeetState();
		return new ParseTree::WhileStatement(whileToken, openParen, condition, closeParen, body);
	}

	inline const ParseTree::ArgumentsNode* argumentList() {
		static constexpr auto isTypename = [](const Token& token) { const Token::Type type = token.type; return type == Token::Type::BOOL || type == Token::Type::INT || type == Token::Type::FLOAT || type == Token::Type::STRING; };

		std::vector<ParseTree::ArgumentsNode::Argument> args;
		std::vector<Token> commas;

		tokenProvider.pushState();

		if(!isTypename(peekToken())) {
			tokenProvider.yeetState();
			return new ParseTree::ArgumentsNode(args, commas);
		}

		for(;;) {
			ParseTree::ArgumentsNode::Argument arg;
			
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
		return new ParseTree::ArgumentsNode(args, commas);
	}

	inline const ParseTree::FunctionDeclarationStatement* functionDeclaration() {
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


		const ParseTree::ArgumentsNode* args = argumentList();
		if(!args) {
			tokenProvider.popState();
			return nullptr;
		}


		if(peekToken().type != Token::Type::PAREN_CLOSE) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& closeParen = getToken(); // consume ')'


		const ParseTree::StatementNode* body = statement();
		if(!body) {
			tokenProvider.popState();
			return nullptr;
		}


		tokenProvider.yeetState();
		return new ParseTree::FunctionDeclarationStatement(typeName, name, openParen, args, closeParen, body);
	}


	// ###############
	// # EXPRESSIONS #
	// ###############
	inline const ParseTree::ExpressionNode* expression() {
		return additiveExpression();
	}
	
	inline const ParseTree::ExpressionNode* additiveExpression() {
		constexpr static auto isSumType = [](const Token::Type type) { return type == Token::Type::PLUS || type == Token::Type::MINUS; };

		const ParseTree::ExpressionNode* a = multiplicativeExpression();
		
		while(isSumType(peekToken().type)) {
			const Token& op = getToken(); // consume operation token
			const ParseTree::ExpressionNode* b = multiplicativeExpression();
			a = new ParseTree::BinaryExpressionNode(a, op, b);
		}

		return a;
	}

	inline const ParseTree::ExpressionNode* multiplicativeExpression() {
		constexpr static auto isMulType = [](const Token::Type& type) { return type == Token::Type::MUL || type == Token::Type::DIV; };

		const ParseTree::ExpressionNode* a = primaryExpression();

		while(isMulType(peekToken().type)) {
			const Token& op = getToken(); // consume operation token
			const ParseTree::ExpressionNode* b = primaryExpression();
			a = new ParseTree::BinaryExpressionNode(a, op, b);
		}

		return a;
	}

	inline const ParseTree::ExpressionNode* primaryExpression() {
		// group:
		if(peekToken().type == Token::Type::PAREN_OPEN) {
			const Token& openParen = getToken(); // consume '('
			const ParseTree::ExpressionNode *expr = expression();
			if(!expr)
				throw std::runtime_error("No Expression inside parentheses");
			if(peekToken().type != Token::Type::PAREN_CLOSE)
				throw std::runtime_error("Missing closing Parenthesis at the end of primary expression");
			const Token& closeParen = getToken(); // consume ')'
			return new ParseTree::GroupExpressionNode(openParen, expr, closeParen);
		}

		// literal:
		if(const ParseTree::LiteralNode* n = literal())
			return n;

		// function call:
		if(const ParseTree::FunctionCallExpressionNode* f = functionCall())
			return f;
		
		// identifier (variable name):
		if(const ParseTree::IdentifierNode* i = identifier())
			return i;
		
		// negation:
		if(peekToken().type == Token::Type::MINUS) {
			const Token& op = getToken(); // consume '-'
			return new ParseTree::UnaryExpressionNode(op, primaryExpression());
		}

		return nullptr;
	}

	inline const ParseTree::FunctionCallExpressionNode* functionCall() {
		// static constexpr auto isTypename = [](const Token& token) { const Token::Type type = token.type; return type == Token::Type::BOOL || type == Token::Type::INT || type == Token::Type::FLOAT || type == Token::Type::STRING; };

		tokenProvider.pushState();

		if(peekToken().type != Token::Type::IDENTIFIER) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& name = getToken(); // consume function name

		if(peekToken().type != Token::Type::PAREN_OPEN) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& openParen = getToken(); // consume '('

		std::vector<const ParseTree::ExpressionNode*> args;
		std::vector<Token> commas;

		const ParseTree::ExpressionNode* expr = expression();
		if(expr) {
			args.push_back(expr); // consume first argument
			
			for(;peekToken().type == Token::Type::COMMA;) {
				commas.push_back(getToken()); // consume ','

				const ParseTree::ExpressionNode* expr = expression();
				if(!expr)
					throw std::runtime_error("Error parsing function call: no expression after comma");
				
				args.push_back(expr); // consume argument
			}
		}

		if(peekToken().type != Token::Type::PAREN_CLOSE) {
			tokenProvider.popState();
			return nullptr;
		}
		const Token& closeParen = getToken(); // consume ')'

		tokenProvider.yeetState();
		return new ParseTree::FunctionCallExpressionNode(name, openParen, args, commas, closeParen);
	}

	inline const ParseTree::IdentifierNode* identifier() {
		if(peekToken().type == Token::Type::IDENTIFIER)
			return new ParseTree::IdentifierNode(getToken());
			
		return nullptr;
	}


	inline const ParseTree::LiteralNode* literal() {
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
			return new ParseTree::LiteralNode(getToken());
		
		return nullptr;
	}
};
