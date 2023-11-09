#pragma once


#include <string_view>
#include <stdexcept>
#include <iostream>
// #include <variant>
#include <cstdint>
#include <vector>
#include <string>


class EvalType {
private:
	std::string type_;

public:
	inline EvalType(const std::string& type): type_(type) {}
	inline const std::string& type() const { return type_; }
};


inline EvalType binaryExpressionType(const EvalType& a, const std::string_view op, const EvalType& b) {
	static const std::vector<std::string> logic_ops { "&&", "||", "==", "!=", "<", ">", "<=", ">=" };

	for(const std::string& op_cur : logic_ops)
		if(op == op_cur)
			return EvalType("bool");

	if(a.type() == b.type()) return a; // base case

	if((a.type() == "int" || b.type() == "int") && (a.type() == "float" || b.type() == "float")) return EvalType("float"); // implicit float conversion

	if(a.type() == "string" || b.type() == "string") return EvalType("string"); // implicit string conversion

	throw std::runtime_error("binaryExpressionType(): invalid combination of types");
}
