#include "stdafx.h"
#include "XMLErr.h"




std::vector<std::string> ErrCodes = { "Ok" ,"File Not Found", "Too Many Declarations","Invalid character found in comment", "Failed to parse nodes", "No declaration declared",
"Tag name may not start with a number", "text must not contain < or > or &. use &amp;, &lt;, &rt, &quot;, or &apos", "Node cannot contain duplicate attribute names",
"Root Node not found"};

std::string XMLErr::GetErrInfo(ERR result)
{
	return ErrCodes.at(static_cast<short>(result));
}