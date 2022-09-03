#pragma once
#include "stdafx.h"

class XMLAttribute
{
private:
	std::string_view name;
	std::string_view value;
public:
	XMLAttribute() noexcept = default;
	XMLAttribute(std::string_view const& attrName, std::string_view const& attrValue) noexcept;
	~XMLAttribute() = default;

	void SetValue(const std::string_view& inValue) { value = inValue; }
	void SetName(const std::string_view& inName) { name = inName; }
	std::string_view GetName() const& { return name; }

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
			return std::string(value);
	}
};