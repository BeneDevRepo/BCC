#pragma once


#include <unordered_map>
#include <iostream>
#include <variant>
#include <cstdint>
#include <string>


struct Symbol {
	enum class Category : uint8_t {
		TYPE, VARIABLE, FUNCTION,
	} category;
	std::string name;
	std::string type;
	inline Symbol(const Category category, const std::string& name, const std::string& type): category(category), name(name), type(type) {}
};

class SymbolTable {
private:
	std::unordered_map<std::string, const Symbol*> symbols;

public:
	inline SymbolTable() {
		declare(new Symbol(Symbol::Category::TYPE, "void", "__VOID__"));
		declare(new Symbol(Symbol::Category::TYPE, "bool", "__BOOL__"));
		declare(new Symbol(Symbol::Category::TYPE, "int", "__INT__"));
		declare(new Symbol(Symbol::Category::TYPE, "float", "__FLOAT__"));
		declare(new Symbol(Symbol::Category::TYPE, "string", "__STRING__"));
	}

	inline void declare(const Symbol *const sym) {
		symbols[sym->name] = sym;
	}

	inline const Symbol* lookup(const std::string& name) const {
		return symbols.contains(name) ? symbols.at(name) : nullptr;
	}

	inline void print() const {
		std::cout << "Symbol Table:\n";
		for(const auto& [name, symbol] : symbols) {
			std::cout << name << ": ";
			std::cout << (symbol->category==Symbol::Category::TYPE ? "<Type>" : symbol->category==Symbol::Category::VARIABLE ? "<Variable>" : "<Function>");
			std::cout << " " << symbol->type << "\n";
		}
	}
};
