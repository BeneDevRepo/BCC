#pragma once


#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>
#include <cctype>

#include "Tokens.hpp"


class Lexer {
public:
	inline std::vector<Token> tokenize(const std::string_view code) {
		std::vector<Token> tokens;

		for(size_t i = 0; i < code.size(); ) {
			const std::string_view remaining = code.substr(i);

			bool found = false;

			for(const TokenDefinition& def : tokenDefinitions) {
				std::match_results<std::string_view::const_iterator> res;

				if(std::regex_search(remaining.cbegin(), remaining.cend(), res, def.regex, std::regex_constants::match_continuous)) {
					if(def.type != Token::Type::SPACE)
						tokens.push_back(Token(def.type, res.str(1), i, (size_t)res.length()));
					i += (size_t)res.length();
					found = true;
					break;
				}
			}

			if(!found)
				throw std::runtime_error{"Invalid Syntax at index " + std::to_string(i)};
		}

		tokens.push_back(Token(Token::Type::END, "EOF"));

		return tokens;
	}
};