#pragma once
#include "stdafx.h"
#include "XMLNode.h"
#include "XMLErr.h"

//auto escapeCharacters = { "\"","'","<",">","&" };

#pragma warning(disable:26812)

enum ReadFlags
{
	None = 0,
	IgnoreDecl = 1,
	IgnoreComments = 2,
	///todo: determine any other readflags
	EndFlags = 4
};

enum WriteFlags
{
	wf_none  = 0
};

enum class bomID
{
	ID_NONE, //undefined, doesn't show in document
	ID_UTF8,
	ID_UTF16,
	ID_UTD32,

	ID_END
};

struct XMLDecl
{
	std::string version = "1.0";
	std::string encoding = "UTF-8";
	bool		isStandalone = false;
};

inline ReadFlags operator|(const ReadFlags& a, const ReadFlags& b) { return static_cast<ReadFlags>(static_cast<int>(a) | static_cast<int>(b)); }
inline ReadFlags operator&(const ReadFlags& a, const ReadFlags& b) { return static_cast<ReadFlags>(static_cast<int>(a) & static_cast<int>(b)); }

class XMLDocument
{
public:
	XMLDocument(ReadFlags flags = ReadFlags::None, WriteFlags writeflags = wf_none ) noexcept;
	~XMLDocument() noexcept;

	void Close();

	std::shared_ptr<XMLDecl>  decl;
private:
	std::unique_ptr<XMLNode> rootNode;
	bool		doesBOMExist;
	bool		ContainsAttributes(std::string_view const& in);
	std::string	BOMString;
	ReadFlags	readFlags;
	WriteFlags  writeFlags;
public:
	//shared between parser/writer
	std::vector<std::string> comments;
	std::unique_ptr<XMLNode> const& GetRootNode() { return rootNode; }
	bool const&    BomExists() { return doesBOMExist; }
	std::string const& GetBOM() { return BOMString; }
//parser functionality
	ERR			   Parse(std::string& text);
	ERR			   ParseDecl(std::string_view& in);
	ERR			   ParseComment(std::string& in);
	ERR			   ParseRootNode(std::string_view& in);
	ERR			   ParseChildNodeRecursively(std::string_view& in, std::unique_ptr<XMLNode> const& parentNode, size_t currPos = 0);
	ERR			   ParseTag(std::string_view const& in, std::unique_ptr<XMLNode> const& parentNode);
	ERR			   ParseAttributes(std::string_view const& in, std::unique_ptr<XMLNode> const& targetNode);
//writer functionality
public:
	void		   CreateDeclaration(const char* version, const char* encoding = 0, bool isStandalone = false);
	std::unique_ptr<XMLNode>	   CreateRootNode(const char* rootName, const char* attrName = 0, const char* attrValue = 0);
	ERR			   WriteNodesRecursively(std::ostream& out, std::unique_ptr<XMLNode> const& node, int indentCount);

	friend std::ostream& operator<<(std::ostream& out, XMLDocument& doc)
	{
		doc.WriteDoc(out);
		return out;
	}
private:
	ERR			   WriteDoc(std::ostream& out);
//end writer functionality
};