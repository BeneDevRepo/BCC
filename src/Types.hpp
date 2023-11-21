#pragma once


#include <string_view>
#include <stdexcept>
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

inline bool isImplicitlyConvertible(const EvalType& src, const EvalType& dst) {
	if(src.type() == dst.type()) return true; // no conversion necessary

	if(src.type() == "bool")
		return dst.type() == "int" || dst.type() == "float" || dst.type() == "string";

	if(src.type() == "int")
		return dst.type() == "bool" || dst.type() == "float" || dst.type() == "string";

	if(src.type() == "float")
		return dst.type() == "string";

	return false;
}

inline EvalType binaryExpressionType(const EvalType& a, const std::string_view op, const EvalType& b) {
	static const std::vector<std::string> logic_ops { "&&", "||", "==", "!=", "<", ">", "<=", ">=" };

	for(const std::string& op_cur : logic_ops) {
		if(op == op_cur) {
			if(!isImplicitlyConvertible(a, EvalType("bool")))
				throw std::runtime_error("binaryExpressionType(): Left-hand-side of binary logic expression is not convertible to bool!");
			if(!isImplicitlyConvertible(b, EvalType("bool")))
				throw std::runtime_error("binaryExpressionType(): Right-hand-side of binary logic expression is not convertible to bool!");

			return EvalType("bool");
		}
	}

	static const std::vector<std::string> convTypes { "int", "float", "string" };
	for(const std::string& type : convTypes)
		if((a.type() == type && isImplicitlyConvertible(b.type(), EvalType(type))) || (b.type() == type && isImplicitlyConvertible(a.type(), EvalType(type))))
			return EvalType(type); // implicit float conversion

	if(a.type() == b.type()) return a; // base case

	throw std::runtime_error("binaryExpressionType(): invalid combination of types");
}
