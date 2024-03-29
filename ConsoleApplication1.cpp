// ConsoleApplication1.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "XMLDocument.h"
#include <chrono>

#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
using namespace std;

int main(int argc, char** argv)
{

	auto startTime = std::chrono::high_resolution_clock::now();
	XMLErr err;

	for (int i = 0; i < 1000; ++i)
	{
		XMLDocument doc(ReadFlags::IgnoreDecl);

		basic_ifstream<char> stream("Mansion.RS.xml", ios::binary);
		if (!stream)
			throw runtime_error(string("cannot open file "));
		stream.unsetf(ios::skipws);

		// Determine stream size
		stream.seekg(0, ios::end);
		size_t size = stream.tellg();
		stream.seekg(0);

		// Load data and add terminating 0
		string m_data = "";
		m_data.resize(size + 1);
		stream.read(&m_data.front(), static_cast<streamsize>(size));
		m_data[size] = 0;

		ERR result = doc.Parse(m_data);

		//if(i == 999)
		//{
		//	std::ofstream file("comparitor.xml");
		//	file << doc;
		//}
		doc.Close();
		stream.close();
		m_data.clear();
	}

	auto endTime = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double,std::ratio<1,1000>> timeResult = endTime - startTime;


	std::cout << timeResult.count() << " milliseconds"  <<  std::endl;

	startTime = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < 1000; ++i)
	{
		rapidxml::file<> file("Mansion.RS.xml");
		rapidxml::xml_document<> doc;
		doc.parse<0x20>(file.data());
		doc.clear();
	}

	endTime = std::chrono::high_resolution_clock::now();

	timeResult = endTime - startTime;


	std::cout << timeResult.count() << " milliseconds rapidxml" << std::endl;

	//system("PAUSE");

	return 0;
}

