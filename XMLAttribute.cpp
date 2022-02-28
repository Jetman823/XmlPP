#include "stdafx.h"
#include "XMLAttribute.h"


std::string numericTypes = {'.','-','e','E','+'};

XMLAttribute::XMLAttribute(std::string_view const& attrName, std::string_view const& attrValue) noexcept
:
name(attrName),
value(attrValue)
{
}