#include "stdafx.h"
#include "XMLAttribute.h"


std::string numericTypes = {'.','-','e','E','+'};

XMLAttribute::XMLAttribute(std::string attrName, std::string attrValue) noexcept
:
name(attrName),
value(attrValue)
{
}