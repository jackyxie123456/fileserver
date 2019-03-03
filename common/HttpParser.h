#pragma once
#include <string>

class HttpParser {
public:
	HttpParser() {

	}
	virtual ~HttpParser() {

	}

	std::string parse(const char* htmlFile);

protected:
	
};

