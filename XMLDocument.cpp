#include "stdafx.h"
#include "XMLDocument.h"
#include "XMLErr.h"
#include <algorithm>
#include <cctype>
#include <functional>
#include <string_view>
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
}

//Parser functionality
ERR XMLDocument::Parse(std::string&  text)
{
	ERR result = ERR::ERR_OK;

	if (text.length() == 0)
		return ERR::ERR_FILE_NOT_FOUND;

	//If flag is set, remove comments from input
	if (readFlags & ReadFlags::IgnoreComments)
	{
		while (text.find("<!--") != std::string::npos)
		{
			size_t first = text.find("<!--");
			size_t last = text.find("-->", first);

			text.erase(first, last - first + 4);
		}
	}

	std::string_view data = text;


	//If flag is not set, parse declaration
	if ((readFlags &~ ReadFlags::IgnoreDecl) == false) 
	{
		result = ParseDecl(data);
		if (result != ERR::ERR_OK)
		{
			return result;
		}
	}
	else
	{
		size_t declend = data.find("?>");
		declend += 2;

		data = data.substr(declend);
	}

	result = ParseRootNode(data);

	return result;
}

ERR XMLDocument::ParseDecl(std::string_view& in)
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

	std::string contents = std::string(in.substr(first + 5, last - (first + 5)));

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

ERR XMLDocument::ParseComment(std::string_view in, std::unique_ptr<XMLNode> const& parentNode)
{
	std::unique_ptr<XMLNode> commentNode = std::make_unique<XMLNode>();
	commentNode->SetName("");
	commentNode->SetInnerText(in);
	commentNode->nodeType = NodeType::NT_COMMENT;
	parentNode->AddChildNode(std::move(commentNode));
	return ERR::ERR_OK;
}

ERR XMLDocument::ParseRootNode(std::string_view& in)
{
	ERR result = ERR::ERR_OK;

	//find opening tag and skip
	size_t first = in.find('<') + 1;

	size_t last = in.find_first_of('>',first);

	if (first == std::string::npos || last == std::string::npos)
	{
		return ERR::ERR_NO_ROOT_NODE_FOUND;
	}

	//if you don't have anything between the opening and closing tag, there can't be a root node. disgregard and return an error
	std::string_view nodeData = in.substr(first, last - first);
	if (nodeData.empty())
	{
		return ERR::ERR_NO_ROOT_NODE_FOUND;
	}

	rootNode = std::make_unique<XMLNode>();

	result = ParseTag(nodeData, rootNode);
	if (result != ERR::ERR_OK)
	{
		return result;
	}

	//scan for and parse any potential child nodes
	size_t find = in.find(std::string_view("</" + std::string(rootNode->GetName())), last);
	if (find != std::string_view::npos)
	{
		std::string_view childNodes = in.substr(last + 1, find - (last + 1));
		result = ParseChildNodeRecursively(childNodes, rootNode);
		if (result != ERR::ERR_OK)
		{
			return result;
		}
	}

	//write doc and compare result to existing xml file; to be deleted at a later date.
	//std::ofstream outDoc("comparitor.xml");
	//WriteDoc(outDoc);


	
	return ERR::ERR_OK;
}

//TODO: optionally parse comment? optimize this entire function.
//note: since we're not erasing any text from the document string, currPos is used to keep track of where we're at in the parsing process
ERR XMLDocument::ParseChildNodeRecursively(std::string_view const in, std::unique_ptr<XMLNode> const& parentNode, size_t currPos)
{
	ERR result = ERR::ERR_OK;

	while (1)
	{
		//search for the first opening tag character. if not found break out of loop
		size_t nodeStart = in.find('<', currPos );
		if (nodeStart == std::string_view::npos)
			break;

		//skip the opening tag character
		++nodeStart;

		//search for end of opening tag
		size_t const nodeEnd = in.find('>', nodeStart);

		if (nodeEnd == std::string_view::npos)
			break;

		std::string_view nodeTag = in.substr(nodeStart, nodeEnd - nodeStart);
		if (nodeTag.empty() == true)
			break;

		//advance to the end of the opening tag
		currPos = nodeEnd + 1;

		if (in.at(nodeStart) == '!')
		{
			nodeTag = in.substr(nodeStart + 3, nodeEnd - (nodeStart + 5));
			result = ParseComment(nodeTag, parentNode);
			continue;
		}



		//initialize childnode and read the entire opening tag name and attributes
		std::unique_ptr<XMLNode> childNode = std::make_unique<XMLNode>();
		childNode->nodeType = NodeType::NT_NORMAL;

		result = ParseTag(nodeTag, childNode);
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

		//NOTE: not supporting mixed content at the moment. if mixed content no guarantees the parser will handle it correctly;
		std::string_view const childData = GetChildData(in, currPos,childNode->GetName());
		if (childData.empty() || childData.find_first_not_of(WHITESPACE) == std::string_view::npos)
		{
			parentNode->AddChildNode(std::move(childNode));
			continue;
		}
		else
		{
			if (childData.find('<') == std::string_view::npos)
			{
				childNode->SetInnerText(childData);
			}

			result = ParseChildNodeRecursively(childData, childNode, 0);
			if (result != ERR::ERR_OK)
			{
				return result;
			}
			parentNode->AddChildNode(std::move(childNode));
		}
	}
	return result;
}

ERR XMLDocument::ParseTag(std::string_view const nodeData, std::unique_ptr<XMLNode> const& targetNode)
{

	ERR result = ERR::ERR_OK;

	//check if node contains no attributes and is simply a flag....
	size_t nameEnd = nodeData.find_first_of(' ');
	//node has no attributes
	if (nameEnd == std::string_view::npos)
	{
		nameEnd = nodeData.find_first_of('/');

		targetNode->SetName(nodeData.substr(0,nameEnd));

		return ERR::ERR_OK;
	}

	/*node may have attributes, or someone may have just put in a ton of spaces for no reason. either way
	it'll still parse the name correctly*/
	targetNode->SetName(nodeData.substr(0,nameEnd));

	//if the tag for the node contains any quotes, attempt to parse attribute data. NOTE: single and double quotes not handled yet
	if (ContainsAttributes(nodeData) == true)
	{
		result = ParseAttributes(nodeData.substr(nameEnd), targetNode);
	}

	return result;
}


ERR XMLDocument::ParseAttributes(std::string_view const in, std::unique_ptr<XMLNode>const& targetNode)
{
	//get a count of how many attributes are contained within the xml node. NOTE: change later, equal sign is allowed within the attributes value
	size_t attrCount = std::count(in.begin(), in.end(), '=');

	size_t currPos = 0;
	for (size_t i = 0; i < attrCount; ++i)
	{

		//get attribute name
		size_t first = in.find_first_of(ALPHABET, currPos);
		size_t last = in.find_first_of("=",first + 1);
		std::string_view const name = in.substr(first, last - first);

		//skip to value
		first = in.find_first_of("\"",currPos) + 1;
		last = in.find_first_of("\"", first + 1);// +first;
		std::string_view const value = in.substr(first, last - first);

		if (name.empty() || value.empty())
		{
			break;
		}

		if (name.find_first_not_of(WHITESPACE) == std::string_view::npos)
		{
			break;
		}


		ERR result = targetNode->AddAttribute(std::make_unique<XMLAttribute>(name, value));
		if (result != ERR::ERR_OK)
		{
			return result;
		}

		currPos = last + 1;

	}

	return ERR::ERR_OK;
}

std::string_view const XMLDocument::GetChildData(std::string_view const in, size_t& currPos, std::string_view const& childName)
{

	size_t foundChildData = in.find(childName, currPos);
	if (foundChildData == std::string_view::npos)
	{
		return "";
	}

	std::string_view data = in.substr(currPos, foundChildData - currPos - 2);
	//advance to end of node and return data
	currPos = foundChildData + 1;

	return data;
}

bool XMLDocument::ContainsAttributes(std::string_view const& in)
{
	if (in.length() == 0)
		return false;

	///Xml Allows for quotes to be apostrophe or quote enclosed
	if (in.find('\"') == std::string_view::npos && in.find('\'') == std::string_view::npos)
		return false;

	return true;
}

//end parser functionality

//Writer functionality : Optional TODO - AddComments function
std::unique_ptr<XMLNode> XMLDocument::CreateRootNode(const char* rootName, const char* attrName, const char* attrValue)
{
	rootNode = std::make_unique<XMLNode>(rootName);

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
		out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
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
ERR XMLDocument::WriteNodesRecursively(std::ostream& in, std::unique_ptr<XMLNode> const& currNode, int indentCount)
{
	for (auto& itor : currNode->GetChildNodes())
	{
		if (itor == nullptr)
			continue;

		if (itor->nodeType == NodeType::NT_COMMENT)
		{
			in << std::string(indentCount,'\t') <<  "<!--" << itor->GetInnerText() << "-->\n";
		}
		else
		{
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
	}
	return ERR::ERR_OK;
}
//end writer functionality