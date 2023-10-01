#pragma once


#include <algorithm>
#include <iostream>
#include <variant>
#include <cstdint>
#include <string>
#include <regex>


struct Span {
private:
	size_t start_, end_;
public:
	inline Span(): start_(0), end_(0) {}
	inline Span(const size_t start, const size_t end): start_(start), end_(end) {}
	inline Span(const Span& a, const Span& b): start_(std::min<size_t>(a.start(), b.start())), end_(std::max<size_t>(a.end(), b.end())) {}
	inline size_t start() const { return start_; }
	inline size_t end() const { return end_; }
	inline size_t len() const { return end_ - start_; }
	inline friend std::ostream& operator<<(std::ostream& cout, const Span& span) { return cout << "[" << span.start() << " " << span.end() << "]"; }
};

struct Token {
	enum class Type : uint8_t {
		VOID, RETURN, IF, WHILE, FOR, DO, SWITCH, CASE, BREAK, CONTINUE,
		
		BOOL, FLOAT, INT, STRING,

		BOOL_LITERAL, INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL,

		SEMICOLON, COMMA, DOT, EQUAL, PLUS, MINUS, MUL, DIV, PAREN_OPEN, PAREN_CLOSE, BRACE_OPEN, BRACE_CLOSE, SQUARE_OPEN, SQUARE_CLOSE,

		IDENTIFIER,

		SPACE, END,
	} type;

	std::string value;

	// size_t start, len;
	Span span;

	inline Token(): type(static_cast<Type>(-1)), value("INVALID TOKEN"), span(static_cast<size_t>(-1), static_cast<size_t>(-1)) {}
	inline Token(const Type type, const std::string& value): type(type), value(value), span() {}
	inline Token(const Type type, const std::string& value, const size_t start, const size_t len): type(type), value(value), span(start, start+len) {}
};

struct TokenProvider {
	inline virtual ~TokenProvider() {}
	inline virtual Token peek() const = 0;
	inline virtual Token consume() = 0;
	inline virtual void pushState() = 0;
	inline virtual void popState() = 0;
	inline virtual void yeetState() = 0;
};

inline std::ostream& operator<<(std::ostream& cout, TokenProvider& lexer) {
	lexer.pushState();

	Token token;

	do {
		token = lexer.consume();
		std::cout << token.value << " ";
		// 	std::cout << "Token " << static_cast<int>(token.type) << " Value: " << token.value << " " << token.span << "\n";
	} while(token.type != Token::Type::END);

	lexer.popState();

	return cout;
}

struct TokenDefinition {
	const std::regex regex;
	const Token::Type type;
};

const thread_local TokenDefinition tokenDefinitions[] {
	// keywords:
	{ std::regex("(void)\\W"), Token::Type::VOID },
	{ std::regex("(return)\\W"), Token::Type::RETURN },
	{ std::regex("(if)\\W"), Token::Type::IF },
	{ std::regex("(while)\\W"), Token::Type::WHILE },
	{ std::regex("(for)\\W"), Token::Type::FOR },
	{ std::regex("(do)\\W"), Token::Type::DO },
	{ std::regex("(switch)\\W"), Token::Type::SWITCH },
	{ std::regex("(case)\\W"), Token::Type::CASE },
	{ std::regex("(break)\\W"), Token::Type::BREAK },
	{ std::regex("(continue)\\W"), Token::Type::CONTINUE },

	// types:
	{ std::regex("(bool)\\W"), Token::Type::BOOL },
	{ std::regex("(int)\\W"), Token::Type::INT },
	{ std::regex("(float)\\W"), Token::Type::FLOAT },
	{ std::regex("(string)\\W"), Token::Type::STRING },

	// literals:
	{ std::regex("(true|false)\\W"), Token::Type::BOOL_LITERAL },
	{ std::regex("([0-9]+\\.[0-9]*|\\.[0-9]+)"), Token::Type::FLOAT_LITERAL },
	{ std::regex("([0-9]+)"), Token::Type::INT_LITERAL },
	{ std::regex("(\"[^\"]*\")"), Token::Type::STRING_LITERAL },

	// operators / symbols:
	{ std::regex("(;)"), Token::Type::SEMICOLON },
	{ std::regex("(,)"), Token::Type::COMMA },
	{ std::regex("(\\.)"), Token::Type::DOT },
	{ std::regex("(=)"), Token::Type::EQUAL },
	{ std::regex("(\\+)"), Token::Type::PLUS },
	{ std::regex("(-)"), Token::Type::MINUS },
	{ std::regex("(\\*)"), Token::Type::MUL },
	{ std::regex("(/)"), Token::Type::DIV },
	{ std::regex("(\\()"), Token::Type::PAREN_OPEN },
	{ std::regex("(\\))"), Token::Type::PAREN_CLOSE },
	{ std::regex("(\\{)"), Token::Type::BRACE_OPEN },
	{ std::regex("(\\})"), Token::Type::BRACE_CLOSE },
	{ std::regex("(\\[)"), Token::Type::SQUARE_OPEN },
	{ std::regex("(\\])"), Token::Type::SQUARE_CLOSE },

	{ std::regex("([a-zA-Z_]\\w*)"), Token::Type::IDENTIFIER },

	{ std::regex("(\\s+)"), Token::Type::SPACE },
};
