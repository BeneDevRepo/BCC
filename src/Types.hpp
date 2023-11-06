#pragma once


#include <iostream>
// #include <variant>
#include <cstdint>
#include <string>


class EvalType {
private:
	std::string type_;
public:
	inline EvalType(const std::string& type): type_(type) {}
};

