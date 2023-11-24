#pragma once


#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>
#include <cctype>

#include "Tokens.hpp"


class Lexer : public TokenProvider {
private:
	std::vector<Token> stack; // contains pushed nextToken values
	const std::string_view source;
	Token nextToken;

public:
	inline Lexer(const std::string_view source): source(source), nextToken(getNextTokenNoSpace(0)) {}

	inline virtual ~Lexer() {
		if(stack.size() > 0)
			std::cout << "ERROR: Tried to destroy Lexer object with non-empty stack\n";
	}

public:
	inline virtual Token peek() const override {
		return nextToken;
	}

	inline virtual Token consume() override {
		const Token res = nextToken;

		nextToken = getNextTokenNoSpace(nextToken.span.end());

		return res;
	}

	inline virtual void pushState() override {
		stack.push_back(nextToken);
	}

	inline virtual void popState() override {
		nextToken = stack.back();
		stack.pop_back();
	}

	inline virtual void yeetState() override {
		stack.pop_back();
	}

private:
	inline Token getNextToken(const size_t ind) const {
		if(ind >= source.size() - 1)
			return Token(Token::Type::END, "EOF");

		std::string_view remainder = source.substr(ind); // unprocessed substring of source

		for(const TokenDefinition& def : tokenDefinitions) {
			std::match_results<std::string_view::const_iterator> res;

			if(std::regex_search(remainder.cbegin(), remainder.cend(), res, def.regex, std::regex_constants::match_continuous))
				return Token(def.type, res.str(1), ind + static_cast<size_t>(res.position(1)), static_cast<size_t>(res.length(1)));
		}

		throw std::runtime_error{"Invalid Syntax at index " + std::to_string(ind)};
	}

	inline Token getNextTokenNoSpace(size_t ind) const {
		Token t;

		do {
			t = getNextToken(ind);
			ind = t.span.end();
		} while(t.type == Token::Type::SPACE);

		return t;
	}
};


class ImmediateLexer : public TokenProvider {
private:
	std::vector<size_t> stack; // contains pushed values of ind
	std::vector<Token> tokens;
	size_t ind;

public:
	inline ImmediateLexer(const std::string_view source): tokens(), ind(0) {
		Lexer lex(source);

		do {
			tokens.push_back(lex.consume());
		} while(tokens.back().type != Token::Type::END);
	}

	inline virtual ~ImmediateLexer() {
		if(stack.size() > 0)
			std::cout << "ERROR: Tried to destroy ImmediateLexer object with non-empty stack\n";
	}

public:
	inline virtual Token peek() const override {
		return tokens[std::min<size_t>(tokens.size()-1, ind)];
	}

	inline virtual Token consume() override {
		return tokens[std::min<size_t>(tokens.size()-1, ind++)];
	}

	inline virtual void pushState() override {
		stack.push_back(ind);
	}

	inline virtual void popState() override {
		ind = stack.back();
		stack.pop_back();
	}

	inline virtual void yeetState() override {
		stack.pop_back();
	}
};
