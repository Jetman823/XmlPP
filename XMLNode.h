#pragma once
#include "stdafx.h"
#include "XMLAttribute.h"
#include "XMLErr.h"

enum NodeType
{
	NT_NORMAL = 0,
	NT_COMMENT = 1,
};


class XMLNode
{
private:
	std::string_view name;
	std::string_view innerText;
	std::vector<std::unique_ptr<XMLAttribute>> attributes;
	std::vector<std::unique_ptr<XMLNode>> childNodes;
	std::vector<std::string_view> comments;
public:
	XMLNode() noexcept = default;
	//Innertext is optional for a node, given that nodes can contain only attributes, or may contain childnodes (if childnodes are contained, there can be no inner text).
	XMLNode(const char* nodeName, const char* nodeInnerText = 0) noexcept;

	~XMLNode() = default;

	void Close();

	void SetName(const std::string_view& value) { name = value; }
	std::string GetName() const& { return std::string(name); }

	void SetInnerText(const std::string_view& value) { innerText = value; }
	std::string GetInnerText() const& { return std::string(innerText); }

	std::vector<std::unique_ptr<XMLAttribute>> const& GetAttributes() const { return attributes; }
	std::vector<std::unique_ptr<XMLNode>> const& GetChildNodes() const { return childNodes; }


	//give users a choice of either adding a pointer to the function, or creating the pointer in the function, then adding the pointer to the node
	ERR AddAttribute(std::unique_ptr<XMLAttribute>&& attribute);
	ERR AddAttribute(const char* attrName, const char* attrValue);
	bool AddChildNode(std::unique_ptr<XMLNode>&& childNode);
	//

	//Comments
	void AddComment(std::string_view in);
	std::vector<std::string_view> const GetCommentsByNode(std::shared_ptr<XMLNode> const& parentNode);
	std::string_view const GetCommentByNodeIndex(int const& index, std::shared_ptr<XMLNode> const& parentNode);
	//

	//check if child exists? idk why you would, but it's easy enough so why not
	bool const ChildExists(std::unique_ptr<XMLNode> const& node);

	XMLAttribute* const GetAttribute(std::string const& attrName);
	XMLAttribute* const GetAttribute(size_t const& index);
	XMLNode* const GetChildNode(size_t const& index);
	bool const FindAttribute(std::string const& attrName);

	NodeType nodeType;

};