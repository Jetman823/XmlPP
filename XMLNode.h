#pragma once
#include "stdafx.h"
#include "XMLAttribute.h"
#include "XMLErr.h"


class XMLNode
{
private:
	std::string name;
	std::string innerText;
	std::vector<std::unique_ptr<XMLAttribute>> attributes;
	std::vector<std::shared_ptr<XMLNode>> childNodes;
	std::vector<std::string> comments;
public:
	XMLNode() noexcept = default;
	//Innertext is optional for a node, given that nodes can contain only attributes, or may contain childnodes (if childnodes are contained, there can be no inner text).
	XMLNode(const char* nodeName, const char* nodeInnerText = 0) noexcept;

	~XMLNode() = default;

	void Close();

	void SetName(const std::string& value) { name = value; }
	std::string GetName() const& { return name; }

	void SetInnerText(const std::string value) { innerText = value; }
	std::string GetInnerText() const& { return innerText; }

	std::vector<std::unique_ptr<XMLAttribute>> const& GetAttributes() const { return attributes; }
	std::vector<std::shared_ptr<XMLNode>> const& GetChildNodes() const { return childNodes; }


	//give users a choice of either adding a pointer to the function, or creating the pointer in the function, then adding the pointer to the node
	ERR AddAttribute(std::unique_ptr<XMLAttribute>&& attribute);
	ERR AddAttribute(const char* attrName, const char* attrValue);
	bool AddChildNode(std::shared_ptr<XMLNode>&& childNode);
	//

	//Comments
	void AddComment(std::string const& in);
	std::vector<std::string> const GetCommentsByNode(std::shared_ptr<XMLNode> const& parentNode);
	std::string const GetCommentByNodeIndex(int const& index, std::shared_ptr<XMLNode> const& parentNode);
	//

	//check if child exists? idk why you would, but it's easy enough so why not
	bool const ChildExists(std::shared_ptr<XMLNode> const& node);

	XMLAttribute* const GetAttribute(std::string const& attrName);
	XMLAttribute* const GetAttribute(size_t const& index);
	XMLNode* const GetChildNode(size_t const& index);
	bool const FindAttribute(std::string const& attrName);

};