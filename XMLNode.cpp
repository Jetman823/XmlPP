#include "stdafx.h"
#include "XMLNode.h"
XMLNode::XMLNode(const char* nodeName, const char* nodeInnerText) noexcept
:
name(nodeName),
innerText(nodeInnerText == nullptr? "" : nodeInnerText)
{
	nodeType = NodeType::NT_NORMAL;
}

///this is called from xmlwriter and xmlreader, delete child node and any child nodes that exists within the instance
void XMLNode::Close()
{
}

ERR XMLNode::AddAttribute(std::unique_ptr<XMLAttribute>&& attribute)
{
	auto result = std::find_if(attributes.begin(), attributes.end(), [&](std::unique_ptr<XMLAttribute> const& attr) {return attribute->GetName() == attr->GetName(); });
	
	//can't have duplicate attributes
	if (result != attributes.end())
	{
		return ERR::ERR_DUPLICATE_ATTRIBUTE;
	}

	attributes.emplace_back(std::move(attribute));

	return ERR::ERR_OK;
}


ERR XMLNode::AddAttribute(const char* attrName, const char* attrValue)
{

	auto result = std::find_if(attributes.begin(), attributes.end(), [&](std::unique_ptr<XMLAttribute> const& attr) {return strcmp(attr->GetName().data(), attrName) == 0; });

	///can't have duplicate attributes
	if (result != attributes.end())
	{
		return ERR::ERR_DUPLICATE_ATTRIBUTE;
	}

	attributes.emplace_back(new XMLAttribute(attrName, attrValue));

	return ERR::ERR_OK;
}


bool XMLNode::AddChildNode(std::unique_ptr<XMLNode>&& childNode)
{
	childNodes.emplace_back(std::move(childNode));
	return true;
}

XMLAttribute* const XMLNode::GetAttribute(std::string const& attrName)
{
	auto attribute = std::find_if(attributes.begin(), attributes.end(), [&](std::unique_ptr<XMLAttribute> const&  attr) {return strcmp(attrName.c_str(), attr->GetName().data()) == 0; });
	if (attribute == attributes.end())
	{
		return nullptr;
	}

	return attribute->get();
}

XMLAttribute* const XMLNode::GetAttribute(size_t const& index)
{
	if (index > attributes.size())
	{
		return nullptr;
	}
	return attributes.at(index).get();
}

XMLNode* const XMLNode::GetChildNode(size_t const& index)
{
	return childNodes.at(index).get(); 
}

bool const XMLNode::ChildExists(std::unique_ptr<XMLNode> const& node)
{
	///you shouldn't be trying to find a nullptr, but doesn't hurt to have a safety check anyways
	if (node == nullptr)
	{
		return false;
	}

	return std::find(childNodes.begin(), childNodes.end(), node) != childNodes.end();
}

bool const XMLNode::FindAttribute(std::string const& attrName)
{
	return std::find_if(attributes.begin(), attributes.end(), [&](std::unique_ptr<XMLAttribute> const& attr) {return strcmp(attr->GetName().data(), attrName.c_str()) == 0; }) != attributes.end();
}