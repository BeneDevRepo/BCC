#pragma once


#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <vector>

#include "Tokens.hpp"



class Parser {
private:
	static constexpr const char* SPACE =  "  ";     // "  " 
	static constexpr const char* VSPACE = "\xB3 ";  // "│ "
	static constexpr const char* VBRANCH = "\xC3\xC4"; // "├─"
	static constexpr const char* LBRANCH = "\xC0\xC4"; // "└─"
	// 0xB3; // 179 │
	// 0xC0; // 192 └
	// 0xC3; // 195 ├
	// 0xC4; // 196 ─
	// └ ┘ ┌ ┐ │ ─ ┤ ├ ┴ ┬ ┼ 

private:
	std::vector<Token> tokens;
	size_t currentToken;

public:
	struct Node {
		inline virtual ~Node() {}
		inline virtual void print(const std::string& indent = "", const bool isLast = true) const { throw std::runtime_error("Tried to print raw Node"); }
	};

	struct ExpressionNode : public Node {
		inline virtual void print(const std::string& indent, const bool isLast) const { throw std::runtime_error("Tried to print raw Expression Node"); }
	};

	struct NegationNode : public ExpressionNode {
		ExpressionNode *a;
		inline NegationNode(ExpressionNode* a): a(a) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << "-" << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, true);
		}
	};

	struct SumNode : public ExpressionNode {
		ExpressionNode *a, *b;
		enum class Type : uint8_t { ADD, SUB } type;
		inline SumNode(ExpressionNode* a, ExpressionNode* b, const Type type): a(a), b(b), type(type) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << (type == Type::ADD ? "+" : "-") << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, false);
			b->print(subIndent, true);
		}
	};

	struct MulNode : public ExpressionNode {
		ExpressionNode *a, *b;
		enum class Type : uint8_t { MUL, DIV } type;
		inline MulNode(ExpressionNode* a, ExpressionNode* b, const Type type): a(a), b(b), type(type) {}

		inline virtual void print(const std::string& indent, const bool isLast) const {
			std::cout << indent << (isLast ? LBRANCH : VBRANCH); // isLast ? "└─" : "├─"
			std::cout << (type == Type::MUL ? "*" : "/") << "\n";

			const std::string subIndent = indent + (isLast ? SPACE : VSPACE); // isLast ? "  " : "│ "
			a->print(subIndent, false);
			b->print(subIndent, true);
		}
	};

	struct VariableNode : public ExpressionNode {
		std::string name;
		inline VariableNode(const std::string& name): name(name) {}
		inline virtual void print(const std::string& indent, const bool isLast) const { std::cout << indent << (isLast ? LBRANCH : VBRANCH) << "<" << name << ">" << "\n"; }
	};

	struct IntLiteralNode : public ExpressionNode {
		int value;
		inline IntLiteralNode(const int value): value(value) {}
		inline virtual void print(const std::string& indent, const bool isLast) const { std::cout << indent << (isLast ? LBRANCH : VBRANCH) << value << "\n"; }
	};

	struct FloatLiteralNode : public ExpressionNode {
		float value;
		inline FloatLiteralNode(const float value): value(value) {}
		inline virtual void print(const std::string& indent, const bool isLast) const { std::cout << indent << value << "\n"; }
	};


public:
	inline Parser() {}
	inline const Token& peekToken() const { return tokens[currentToken]; }
	inline const Token& getToken() { return tokens[currentToken++]; }


	// inline VariableDeclarationNode* variableDeclaration() {
	// 	static constexpr auto isTypename = [](const Token& token) { const Token::Type type = token.type; return type == Token::Type::BOOL || type == Token::Type::INT || type == Token::Type::FLOAT || type == Token::Type::STRING; };

	// 	const size_t currentTokenBefore = currentToken;

	// 	if(!isTypename(peekToken()))
	// 		return nullptr;
		
	// 	const Token type = getToken();

	// 	if(peekToken().type != Token::Type::EQUAL) {
	// 		currentToken = currentTokenBefore;
	// 		return nullptr;
	// 	}
		

			
	// 	return nullptr;
	// }

	// inline TypenameNode* typeName() {
	// }

	inline ExpressionNode* expression() {
		return additiveExpression();
	}
	
	inline ExpressionNode* additiveExpression() {
		constexpr static auto isSumType = [](const Token::Type type) { return type == Token::Type::PLUS || type == Token::Type::MINUS; };

		ExpressionNode* a = multiplicativeExpression();
		
		while(isSumType(peekToken().type)) {
			Token::Type opType = getToken().type; // consume operation token
			SumNode::Type op = opType == Token::Type::PLUS ? SumNode::Type::ADD : SumNode::Type::SUB;

			ExpressionNode* b = multiplicativeExpression();

			a = new SumNode(a, b, op);
		}

		return a;
	}

	inline ExpressionNode* multiplicativeExpression() {
		constexpr static auto isMulType = [](const Token::Type type) { return type == Token::Type::MUL || type == Token::Type::DIV; };

		ExpressionNode* a = primaryExpression();

		while(isMulType(peekToken().type)) {
			Token::Type opType = getToken().type; // consume operation token
			MulNode::Type op = opType == Token::Type::MUL ? MulNode::Type::MUL : MulNode::Type::DIV;

			ExpressionNode* b = primaryExpression();

			a = new MulNode(a, b, op);
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

		// number literal:
		if(ExpressionNode* n = number())
			return n;
		
		// variable name:
		if(ExpressionNode* v = variable())
			return v;
		
		// negation:
		if(peekToken().type == Token::Type::MINUS) {
			getToken(); // consume '-'
			return new NegationNode(primaryExpression());
		}

		return nullptr;
	}

	inline VariableNode* variable() {
		if(peekToken().type == Token::Type::IDENTIFIER)
			return new VariableNode(getToken().value);
			
		return nullptr;
	}


	inline ExpressionNode* number() {
		if(peekToken().type == Token::Type::INT_LITERAL)
			return new IntLiteralNode(std::stoi(getToken().value));

		if(peekToken().type == Token::Type::FLOAT_LITERAL)
			return new FloatLiteralNode(std::stof(getToken().value));
		
		return nullptr;
	}

	inline Node* parse(const std::vector<Token>& tokens) {
		this->tokens = tokens;
		this->currentToken = 0;
		
		ExpressionNode* tree = expression();

		return tree;
	}
};
