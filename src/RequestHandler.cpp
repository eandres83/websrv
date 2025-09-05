#include "../includes/RequestHandler.hpp"

// Funcion principal que gestiona todo los metodos
Response RequestHandler::handle(const Request& request, const Config& config)
{
	if (request.getMethod() == "GET")
		return (_handleGet(request, config));
	else if (request.getMethod() == "POST")
		return (_handlePost(request, config));
	else if (request.getMethod() == "DELETE")
		return (_handleDelete(request, config));

	Response response;
	response.setStatusCode("501", "Not Implemented");
	response.setBody("<html><body><h1>501 Not Implemented</h1></body></html>");
	response.addHeader("Content-Type", "text/html");
	return (response);
}

// Implementacion de los metodos

Response RequestHandler::_handleGet(const Request& request, const Config& config)
{
	std::string path = request.getPath();
	if (path == "/")
		path = "/index.html";

	std::string full_path = config.getRootDirectory() + path;
	Response response;

	// Comprobar si el archivo existe
	if (access(full_path.c_str(), F_OK) == -1)
	{
		response.setStatusCode("404", "Not Found");
		std::string error_body = _readFile(config.getErrorPage404());
		response.setBody(error_body.empty() ? "404 Not Found" : error_body);
	}
	else
	{
		std::string body = _readFile(full_path);
		response.setStatusCode("200", "OK");
		response.setBody(body);
	}
	response.addHeader("Content-Type", "text/html");
	return (response);
}

Response RequestHandler::_handlePost(const Request& request, const Config& config)
{
	// Construimos la ruta donde se guardara el archivo
	// Ej: si la petición es a /nuevo, la ruta sera ./www/nuevo
	std::string full_path = config.getRootDirectory() + request.getPath();
	Response response;

	// Comprobamos si el recurso ya existe para evitar sobrescribirlo
	if (access(full_path.c_str(), F_OK) == 0)
	{
		response.setStatusCode("409", "Conflict"); // 409 Conflict es apropiado si el recurso ya existe
		response.setBody("<html><body><h1>Error 409: El recurso ya existe.</h1></body></html>");
		response.addHeader("Content-Type", "text/html");
		return (response);
	}

	// Abrimos un archivo para escribir en el
	std::ofstream new_file(full_path.c_str());
	if (!new_file.is_open())
	{
		// Si no se pudo crear el archivo (ej. por permisos)
		response.setStatusCode("500", "Internal Server Error");
		response.setBody("<html><body><h1>Error 500: No se pudo crear el recurso.</h1></body></html>");
		response.addHeader("Content-Type", "text/html");
		return (response);
	}

	// Escribimos el cuerpo de la peticion en el archivo
	new_file << request.getBody();
	new_file.close();

	// Si todo fue bien devolvemos un exito
	response.setStatusCode("201", "Created");
	response.setBody("<html><body><h1>Recurso Creado</h1></body></html>");
	response.addHeader("Content-Type", "text/html");
	return (response);
}

Response RequestHandler::_handleDelete(const Request& request, const Config& config)
{
	std::string full_path = config.getRootDirectory() + request.getPath();
	Response response;

	if (remove(full_path.c_str()) == 0)
	{
		response.setStatusCode("200", "OK");
		response.setBody("<html><body><h1>Recurso eliminado</h1></body></html>");
	}
	else
	{
		response.setStatusCode("404", "Not Found");
		response.setBody("<html><body><h1>Error al eliminar el recurso</h1></body></html>");
	}
	response.addHeader("Content-Type", "text/html");
	return (response);
}

std::string RequestHandler::_readFile(const std::string& path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return ("");
	std::stringstream buffer;
	buffer << file.rdbuf();
	return (buffer.str());
}

