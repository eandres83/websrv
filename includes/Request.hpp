#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <iostream>
#include <map>

class Request
{
	private:
		std::string _method;
		std::string _path;
		std::string _body;

	public:
		Request();
		~Request();

		// El "parse" simulado. Recibe el buffer solo para mantener la estructura.
		void parse(const std::string& raw_request);

		// getters
		const std::string& getMethod() const;
		const std::string& getPath() const;
		const std::string& getBody() const;

};

#endif // REQUEST_HPP
