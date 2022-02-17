#pragma once
#include "stdafx.h"

enum class ERR
{
	ERR_OK,
	ERR_FILE_NOT_FOUND,
	ERR_FATAL_DECL_AFTER_BEGINNING,
	ERR_FATAL_INVALID_COMMENT_CHAR,
	ERR_PARSING_NODES,
	ERR_FATAL_NO_DECL_DECLARED,
	ERR_FATAL_TAGNAME_FIRST_NUM,
	ERR_FATAL_INVALID_XML_CHAR,
	ERR_DUPLICATE_ATTRIBUTE,
	ERR_NO_ROOT_NODE_FOUND,
	ERR_UNKNOWN,
	ERR_END
};

class XMLErr
{
public:
	static std::vector<std::string> Errors;
public:
	static std::string GetErrInfo(ERR result);
};