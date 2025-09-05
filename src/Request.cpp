#include "../includes/Request.hpp"

Request::Request()
{
	std::cout << "Request constructor" << std::endl;
}

Request::~Request()
{
	std::cout << "Request destructor" << std::endl;
}

void	Request::parse(const std::string& raw_request)
{
	size_t method_end = raw_request.find(" ");
	if (method_end != std::string::npos)
	{
		_method = raw_request.substr(0, method_end);
		size_t path_end = raw_request.find(" ", method_end + 1);
		if (path_end != std::string::npos)
			_path = raw_request.substr(method_end + 1, path_end - (method_end + 1));
	}

	size_t body_start = raw_request.find("\r\n\r\n");
	if (body_start != std::string::npos)
		_body = raw_request.substr(body_start + 4);
}

const std::string& Request::getMethod() const
{
	return (_method);
}

const std::string& Request::getPath() const
{
	return (_path);
}

const std::string& Request::getBody() const
{
	return (_body);
}

