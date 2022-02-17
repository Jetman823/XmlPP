#pragma once
#include "stdafx.h"

class XMLAttribute
{
private:
	std::string name;
	std::string value;
public:
	XMLAttribute() noexcept = default;
	XMLAttribute(std::string attrName, std::string attrValue) noexcept;
	~XMLAttribute() = default;

	void SetValue(const std::string& inValue) { value = inValue; }
	void SetName(const std::string& inName) { name = inName; }
	std::string GetName() const& { return name; }

	template<typename T>
	T GetValue()
	{
		if constexpr (std::is_arithmetic_v<T>)
		{
			std::stringstream ss(value);
			T ret{};
			ss >> ret;
			return ret;
		}
		else
			return value;
	}
};