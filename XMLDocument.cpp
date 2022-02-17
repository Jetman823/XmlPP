#include "stdafx.h"
#include "XMLDocument.h"
#include "XMLErr.h"
#include <algorithm>
#include <cctype>
#include <functional>
constexpr auto ALPHABET = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
constexpr auto WHITESPACE = " \t\n\v\f\r";

#pragma warning(disable:26812)

XMLDocument::XMLDocument(ReadFlags readflags, WriteFlags writeflags) noexcept
:
decl(nullptr),
rootNode(nullptr),
doesBOMExist(false),
BOMString(""),
readFlags(readflags),
writeFlags(writeflags)
{

}

XMLDocument::~XMLDocument() noexcept
{

}

void XMLDocument::Close()
{
	rootNode.reset();
	decl.reset();
}

//Parser functionality
ERR XMLDocument::Parse(std::string&  text)
{
	ERR result = ERR::ERR_OK;

	if (text.length() == 0)
		return ERR::ERR_FILE_NOT_FOUND;

	//If flag is set, remove comments from input
	/*if (readFlags & ReadFlags::IgnoreComments)
	{
		while (text.find("<!--") != std::string::npos)
		{
			size_t first = text.find("<!--");
			size_t last = text.find("-->", first);

			text.erase(first, last - first + 4);
		}
	}*/

	//If flag is not set, parse declaration
	if (readFlags &~ ReadFlags::IgnoreDecl) 
	{
		result = ParseDecl(text);
		if (result != ERR::ERR_OK)
		{
			return result;
		}
	}

	result = ParseRootNode(text);

	return result;
}

ERR XMLDocument::ParseDecl(std::string& in)
{
	decl = std::make_shared<XMLDecl>();

	size_t first = in.find("<?xml");
	if (first == std::string::npos)
	{
		return ERR::ERR_FATAL_NO_DECL_DECLARED;
	}
	size_t last = in.find("?>");
	if (last == std::string::npos)
	{
		return ERR::ERR_FATAL_NO_DECL_DECLARED;
	}

	std::string contents = in.substr(first + 5,last - (first + 5));

	first = contents.find("version=\"");
	if (first != std::string::npos)
	{

		size_t end = contents.find("\"", first + 9);
		decl->version = contents.substr(first + 9, end  - (first + 9));
	}

	first = contents.find("encoding=\"");
	if (first != std::string::npos)
	{
		size_t end = contents.find("\"", first + 10);
		decl->encoding = contents.substr(first + 10, end - (first + 10));
	}

	first = contents.find("isstandalone=\"");
	if (first != std::string::npos)
	{
		size_t end = contents.find("\"", first + 14);
		std::string value = contents.substr(first + 14, end - (first + 14));
		std::transform(value.begin(), value.end(),value.begin(), ::tolower);
		if (value == "true")
		{
			decl->isStandalone = true;
		}
		else
		{
			decl->isStandalone = false;
		}
	}
	else
	{
		decl->isStandalone = false;
	}

	in = in.substr(last + 3);

	return ERR::ERR_OK;
}

ERR XMLDocument::ParseComment(std::string& in)
{
	while (in.find("<!--") != std::string::npos)
	{
		size_t first = in.find("<!--");
		size_t last = in.find("-->");
		std::string comment = in.substr(first + 4, last - (first + 4));
		if (comment.find("--") != std::string::npos)
		{
			return ERR::ERR_FATAL_INVALID_COMMENT_CHAR;
		}
		comments.push_back(comment);
		in.erase(first, last + 3 - first);

	}

	comments.push_back(in);
	return ERR::ERR_OK;
}

#include <string_view>
ERR XMLDocument::ParseRootNode(std::string& in)
{
	ERR result = ERR::ERR_OK;

	std::string_view data = in;

	size_t first = in.find("<");
	size_t last = in.find_first_of(">");

	if (first == std::string::npos || last == std::string::npos)
	{
		return ERR::ERR_NO_ROOT_NODE_FOUND;
	}

	//if you don't have anything between the opening and closing tag, there can't be a root node. disgregard and return an error
	std::string_view nodeData = data.substr(first + 1, last - (first + 1));
	if (nodeData.empty())
	{
		return ERR::ERR_NO_ROOT_NODE_FOUND;
	}

	rootNode = std::make_shared<XMLNode>();

	result = ParseTag(nodeData, rootNode);
	if (result != ERR::ERR_OK)
	{
		return result;
	}

	//scan for and parse any potential child nodes
//	std::string needle = "</" + rootNode->GetName();
	size_t find = data.find(std::string_view("</" + rootNode->GetName()), last);
	if (find != std::string_view::npos)
	{
		std::string_view childNodes = data.substr(last + 1, find - (last + 1));//in.find(std::string("</" + rootNode->GetName())) - (last + 1));
		result = ParseChildNodeRecursively(childNodes, rootNode);
		if (result != ERR::ERR_OK)
		{
			return result;
		}
	}


	
	return ERR::ERR_OK;
}

//TODO: optionally parse comment? optimize this entire function.
//note: since we're not erasing any text from the document string, currPos is used to keep track of where we're at in the parsing process
ERR XMLDocument::ParseChildNodeRecursively(std::string_view& in, std::shared_ptr<XMLNode> const& parentNode, size_t&& currPos)
{
	ERR result = ERR::ERR_OK;

	while (1)
	{

		size_t nodeStart = in.find_first_of('<', currPos);
		size_t nodeEnd = in.find_first_of('>', nodeStart);

		if (nodeStart == std::string_view::npos || nodeEnd == std::string_view::npos)
			break;

		std::string_view nodeData = in.substr(nodeStart + 1, nodeEnd - (nodeStart + 1));
		if (nodeData.empty() == true)
			break;

		std::shared_ptr<XMLNode> childNode = std::make_shared<XMLNode>();

		result = ParseTag(nodeData, childNode);
		if (result != ERR::ERR_OK)
		{
			return result;
		}

		//if node has closing tag, add and continue.
		if (in.at(nodeEnd - 1) == '/')
		{
			parentNode->AddChildNode(std::move(childNode));
			currPos = nodeEnd + 1;
			continue;
		}

		//otherwise, check for childnodes and parse accordingly
		//todo: optimize this!!!
		size_t find = in.find(std::string("</" + childNode->GetName()), nodeEnd);
		if (find != std::string_view::npos)
		{
			//Not supporting mixexd content, so no need to handle its logic
			std::string_view childNodes = in.substr(nodeEnd + 1, find - (nodeEnd + 1));
			if (childNodes.find_first_not_of(WHITESPACE) == std::string_view::npos)
			{
				currPos = find + childNodes.length();
				parentNode->AddChildNode(std::move(childNode));
				continue;
			}

			if (childNodes.find('<') == std::string_view::npos)
			{
				childNode->SetInnerText(std::string(childNodes));
			}
			else
			{
				result = ParseChildNodeRecursively(childNodes, childNode);
				if (result != ERR::ERR_OK)
					break;
			}
			currPos = find + std::string_view("</" + childNode->GetName()).length();// length();
			parentNode->AddChildNode(std::move(childNode));
			continue;
		}
		else
			break;
	}
	return result;
}

ERR XMLDocument::ParseTag(std::string_view& nodeData, std::shared_ptr<XMLNode> const& targetNode)
{

	ERR result = ERR::ERR_OK;

	//check if node contains no attributes and is simply a flag....
	size_t nameEnd = nodeData.find_first_of(' ');
	//node has no attributes
	if (nameEnd == std::string_view::npos)
	{
		nameEnd = nodeData.find_first_of('/');
		std::string name = std::string(nodeData.substr(0, nameEnd));
		
		targetNode->SetName(name);

		return ERR::ERR_OK;
	}

	/*node may have attributes, or someone may have just put in a ton of spaces for no reason. either way
	it'll still parse the name correctly*/
	std::string nodeName = std::string(nodeData.substr(0, nameEnd));

	targetNode->SetName(nodeName);

	if (nodeData.find_first_of('\'') != std::string_view::npos || nodeData.find_first_of('"') != std::string_view::npos)
	{

		std::string_view attributes = nodeData.substr(nodeData.find_first_of(' '));
		result = ParseAttributes(attributes, targetNode);
	}

	return result;
}


///TODO: handle logic correctly, account for people making mistakes???
ERR XMLDocument::ParseAttributes(std::string_view& in, std::shared_ptr<XMLNode>const& targetNode)
{
	size_t pos = 0;
	while (pos != in.length())
	{

		size_t nameFirst = in.find_first_not_of(WHITESPACE,pos);
		if (nameFirst == std::string_view::npos)
			break;
		size_t nameLast = in.find('=', nameFirst + 1);
		if (nameLast == std::string_view::npos)
			break;

		std::shared_ptr<XMLAttribute> attr = std::make_shared<XMLAttribute>();


		std::string attrName = std::string(in.substr(nameFirst, nameLast - nameFirst));

		attr->SetName(attrName);

		size_t attrFirst = in.find("\"", nameLast);
		size_t attrLast = in.find("\"", attrFirst + 1);

		std::string attrValue = std::string(in.substr(attrFirst + 1, attrLast - (attrFirst + 1)));
		attr->SetValue(attrValue);

		ERR result = targetNode->AddAttribute(std::move(attr));
		if (result != ERR::ERR_OK)
		{
			return result;
		}
		pos = attrLast + 1;
	}
	return ERR::ERR_OK;
}

bool XMLDocument::ContainsAttributes(std::string& in)
{
	if (in.length() == 0)
		return false;

	///Xml Allows for quotes to be apostrophe or quote enclosed
	if (in.find("\"") == std::string::npos && in.find("'") == std::string::npos)
		return false;

	return true;
}

//end parser functionality

//Writer functionality : Optional TODO - AddComments function
std::shared_ptr<XMLNode> XMLDocument::CreateRootNode(const char* rootName, const char* attrName, const char* attrValue)
{
	rootNode = std::make_shared<XMLNode>(rootName);

	if (attrName != nullptr && attrValue != nullptr)
		rootNode->AddAttribute(attrName, attrValue);

	return std::move(rootNode);
}

void XMLDocument::CreateDeclaration(const char* version, const char* encoding, bool isStandalone)
{
	decl = std::make_shared<XMLDecl>();
	decl->version = version;
	decl->encoding = encoding;
	decl->isStandalone = isStandalone;
}

///this is used internally by xmldocument, the overloaded cin operator uses this to write the document
ERR XMLDocument::WriteDoc(std::ostream&out)
{
	if (decl != nullptr)
	{
		out <<	"<?xml version=" << "\"" << decl->version << "\" ";
		if (decl->encoding.length() > 0)
			out << "encoding=" << "\"" << decl->encoding << "\" ";
		if (decl->isStandalone)
			out << "standalone=" << "\"" << "true" << "\"";

		out << "?>" << "\n";
	}
	//if no decl declared, just set a default
	else 
	{
		out << "<?xml version=\"1.0\" encoding=\"UTF-8\"";
	}

	char firstLetter = rootNode->GetName().at(0);
	if (::isdigit(firstLetter))
	{
		return ERR::ERR_FATAL_TAGNAME_FIRST_NUM;
	}

	out << "<" << rootNode->GetName();
	for (auto& it : GetRootNode()->GetAttributes())
	{
		out << " " << it->GetName() << "=" << "\"" << it->GetValue<std::string>() << "\"";
	}
	out << ">\n";
	int indentCount = 1;


	WriteNodesRecursively(out, GetRootNode(), indentCount);

	out << "</" << rootNode->GetName() << ">";

	return ERR::ERR_OK;
}

///TODO: try and insert comments specifically where user wants them, rather than just inserting them all inside the node at one spot.
ERR XMLDocument::WriteNodesRecursively(std::ostream& in, std::shared_ptr<XMLNode> const& currNode, int indentCount)
{
	for (auto& itor : currNode->GetChildNodes())
	{
		if (itor == nullptr)
			continue;
		char firstLetter = itor->GetName().at(0);
		if (::isdigit(firstLetter))
		{
			return ERR::ERR_FATAL_TAGNAME_FIRST_NUM;
		}

		in << std::string(indentCount, '\t') << "<" << itor->GetName();
		for (auto& it : itor->GetAttributes())
		{
			in << " " << it->GetName() << "=" << "\"" << it->GetValue<std::string>() << "\"";
		}

		if (itor->GetInnerText().length() > 0 && itor->GetChildNodes().size() == 0)
		{
			in << ">";
			in << itor->GetInnerText();
			in << "</" << itor->GetName() << ">" << "\n";
		}
		if (itor->GetInnerText().length() == 0 && itor->GetChildNodes().size() == 0 && itor->GetAttributes().size() > 0)
		{
			in << "/>" << "\n";
		}
	    if (itor->GetChildNodes().size() > 0)
		{
			in << ">\n";
			++indentCount;

			WriteNodesRecursively(in, itor, indentCount);
			in << std::string(indentCount > 0 ? --indentCount : 0, '\t') << "</" << itor->GetName() << ">" << "\n";
		}

		//maybe someone uses this to write a node that woudl just indicate a flag to enable/disable something in their program
		if (itor->GetInnerText().length() == 0 && itor->GetChildNodes().size() == 0 && itor->GetAttributes().size() == 0)
		{
			in << "/>" << "\n";
		}
	}
	return ERR::ERR_OK;
}
//end writer functionality