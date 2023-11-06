#pragma once


#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <variant>
#include <cstdint>
#include <string>


namespace AST { struct Node; };

struct Symbol {
	enum class Category : uint8_t {
		TYPE, VARIABLE, FUNCTION,
	} category;
	using Type = std::variant<const std::string, const AST::Node*>;
	std::string name;
	Type type;
	inline Symbol(const Category category, const std::string& name, const std::string& type): category(category), name(name), type(type) {}
	inline Symbol(const Category category, const std::string& name, const AST::Node* type): category(category), name(name), type(type) {}
};

class ScopedSymbolTable {
private:
	std::string scopeName;
	std::unordered_map<std::string, const Symbol*> symbols;

public:
	ScopedSymbolTable* parent;

public:
	inline ScopedSymbolTable(const std::string& scopeName, ScopedSymbolTable* parent = nullptr): scopeName(scopeName), parent(parent) {
	}

	inline void declare(const Symbol *const sym) {
		if(lookup(sym->name))
			throw std::runtime_error("ScopedSymbolTable::declare(): Tried to redeclare symbol \"" + sym->name + "\"");

		symbols[sym->name] = sym;
	}

	inline void overwrite(const Symbol *const sym) {
		if(!lookup(sym->name))
			throw std::runtime_error("ScopedSymbolTable::overwrite(): Tried to overwrite non-existant symbol \"" + sym->name + "\"");
		if(lookup(sym->name)->category != sym->category)
			throw std::runtime_error("ScopedSymbolTable::overwrite(): Tried to overwrite symbol of different categories \"" + sym->name + "\"");

		symbols[sym->name] = sym;
	}

	inline const Symbol* lookup(const std::string& name) const {
		return symbols.contains(name) ? symbols.at(name) : nullptr;
	}

	inline const Symbol* lookupRecursive(const std::string& name) const {
		for(const ScopedSymbolTable* table = this; table != nullptr; table = table->parent)
			if(table->symbols.contains(name))
				return table->symbols.at(name);
		return nullptr;
	}

	inline void print() const {
		std::cout << "Symbol Table:\n";
		for(const auto& [name, symbol] : symbols) {
			std::cout << name << ": ";
			std::cout << (symbol->category==Symbol::Category::TYPE ? "<Type>" : symbol->category==Symbol::Category::VARIABLE ? "<Variable>" : "<Function>");
			std::cout << " ";

			if(std::holds_alternative<const std::string>(symbol->type))
				std::cout << std::get<const std::string>(symbol->type);

			if(std::holds_alternative<const AST::Node*>(symbol->type))
				std::cout << "<AST::Node*>";

			std::cout << "\n";
		}
	}
};
