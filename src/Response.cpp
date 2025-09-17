#include "../includes/Response.hpp"
#include "../includes/Server.hpp"

Response::Response(): _http_version("HTTP/1.1")
{
}

Response::~Response()
{
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

void Response::buildSimpleResponse(const std::string& code, const std::string& message)
{
	setStatusCode(code, message);

	std::stringstream body_stream;
	body_stream << "<html><head><title>" << code << " " << message << "</title></head>"
		<< "<body><h1>" << code << " " << message << "</h1></body></html>";

	setBody(body_stream.str());

	addHeader("Content-Type", "text/html");
}

void Response::buildCustomResponse(const std::string& code, const std::string& message, const std::string& body)
{
	setStatusCode(code, message);
	setBody(body);
}

// Función auxiliar para leer archivos (puedes mover la que tienes en MethodHandler aquí)
static std::string readFile(const std::string& path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return ("");
	std::stringstream buffer;
	buffer << file.rdbuf();
	return (buffer.str());
}

// Mapa de mensajes de estado estandar
static std::map<int, std::string> getStatusMessages()
{
	std::map<int, std::string> messages;
	messages[400] = "Bad Request";
	messages[403] = "Forbidden";
	messages[404] = "Not Found";
	messages[405] = "Method Not Allowed";
	messages[413] = "Payload Too Large";

	messages[500] = "Internal Server Error";
	messages[501] = "Not Implemented";

	// ... añade mas si los necesito
	return (messages);
}

void Response::buildErrorResponse(int error_code, const ServerConfig& config)
{
	std::map<int, std::string> status_messages = getStatusMessages();
	std::string message = status_messages.count(error_code) ? status_messages[error_code] : "Error";

	std::string body;
	// 1. Comprobar si hay una pagina personalizada en la config
	if (config.error_pages.count(error_code))
	{
		body = readFile(config.error_pages.at(error_code));
	}

	// 2. Si no hay pagina personalizada o no se pudo leer, generar una por defecto
	if (body.empty())
	{
		std::stringstream ss;
		ss << error_code;
		buildSimpleResponse(ss.str(), message);
	}
	else // 3. Si se leyo la pagina personalizada, usarla
	{
		std::stringstream ss;
		ss << error_code;
		buildCustomResponse(ss.str(), message, body);
		addHeader("Content-Type", "text/html");
	}
}

