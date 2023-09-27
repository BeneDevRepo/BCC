#pragma once


#include <variant>
#include <cstdint>
#include <string>
#include <regex>


struct Token {
	enum class Type : uint8_t {
		VOID, BOOL, FLOAT, INT, STRING, RETURN,

		BOOL_LITERAL, INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL,

		SEMICOLON, COMMA, DOT, EQUAL, PLUS, MINUS, MUL, DIV, PAREN_OPEN, PAREN_CLOSE, BRACE_OPEN, BRACE_CLOSE, SQUARE_OPEN, SQUARE_CLOSE,

		IDENTIFIER,
		
		SPACE, END,
	} type;

	std::string value;

	size_t start, len;

	inline Token(const Type type): type(type), value(""), start(0), len(0) {}
	inline Token(const Type type, const std::string& value): type(type), value(value), start(0), len(0) {}
	inline Token(const Type type, const std::string& value, const size_t start, const size_t len): type(type), value(value), start(start), len(len) {}
};

struct TokenDefinition {
	const std::regex regex;
	const Token::Type type;
};

const thread_local TokenDefinition tokenDefinitions[] {
	{ std::regex("(void)"), Token::Type::VOID },
	{ std::regex("(bool)"), Token::Type::BOOL },
	{ std::regex("(int)"), Token::Type::INT },
	{ std::regex("(float)"), Token::Type::FLOAT },
	{ std::regex("(string)"), Token::Type::STRING },
	{ std::regex("(return)"), Token::Type::RETURN },

	{ std::regex("(true|false)"), Token::Type::BOOL_LITERAL },
	{ std::regex("([0-9]+\\.[0-9]*|\\.[0-9]+)"), Token::Type::FLOAT_LITERAL },
	{ std::regex("([0-9]+)"), Token::Type::INT_LITERAL },
	{ std::regex("\"([^\"]*)\""), Token::Type::STRING_LITERAL },

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

	{ std::regex("([a-zA-Z_][a-zA-Z0-9_]*)"), Token::Type::IDENTIFIER },

	{ std::regex("(\\s+)"), Token::Type::SPACE },
};
