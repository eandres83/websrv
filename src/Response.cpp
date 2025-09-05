#include "../includes/Response.hpp"
#include "../includes/Server.hpp"

Response::Response(): _http_version("HTTP/1.1")
{
	std::cout << "Default Response constructor called" << std::endl;
}

Response::~Response()
{
	std::cout << "Response destructor called" << std::endl;
}

void	Response::setStatusCode(const std::string& code, const std::string& message)
{
	_status_code = code;
	_status_message = message;
}

void	Response::addHeader(const std::string& key, const std::string& value)
{
	_headers[key] = value;
}

void	Response::setBody(const std::string& body)
{
	_body = body;
	std::stringstream ss;
	ss << body.length();
	addHeader("Content-Length", ss.str());
}

std::string Response::toString() const
{
	std::stringstream response_stream;

	// Status line
	response_stream << _http_version << " " << _status_code << " " << _status_message << "\r\n";

	// Headers
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
	{
		response_stream << it->first << ": " << it->second << "\r\n";
	}

	// Separador
	response_stream << "\r\n";

	// Body
	response_stream << _body;

	return (response_stream.str());
}

