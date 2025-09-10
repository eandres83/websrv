#include "../includes/MethodHandler.hpp"

// Funcion principal que gestiona todo los metodos
Response MethodHandler::handle(const Request& request, const Config& config)
{
	if (request.getMethod() == "GET")
		return (_handleGet(request, config));
	else if (request.getMethod() == "POST")
		return (_handlePost(request, config));
	else if (request.getMethod() == "DELETE")
		return (_handleDelete(request, config));

	Response response;
	response.buildSimpleResponse("501", "Not Implemented");
	return (response);
}

// --- Implementacion de los metodos --- 

Response MethodHandler::_handleGet(const Request& request, const Config& config)
{
	std::string full_path = config.getRootDirectory() + request.getPath();
	Response response;

	if (access(full_path.c_str(), F_OK) == -1) // Si el archivo no existe
	{
		response.setStatusCode("404", "Not Found");
		std::string error_body = _readFile(config.getErrorPage404());
		if (error_body.empty())
			response.setBody("404 Not Found");
		else
			response.setBody(error_body);
		response.addHeader("Content-Type", "text/html");
	}
	else // Si el archivo existe
	{
		response.buildCustomResponse("200", "OK", _readFile(full_path));
		response.addHeader("Content-Type", _getMimeType(full_path));
	}
	return (response);
}

Response MethodHandler::_handlePost(const Request& request, const Config& config)
{
	// Construimos la ruta donde se guardara el archivo
	// Ej: si la petición es a /nuevo, la ruta sera ./www/nuevo
	// Porque el rootDirecotry es ./www que se define en el archivo de confg
	std::string full_path = config.getRootDirectory() + request.getPath();
	Response response;

	// Comprobamos si el recurso ya existe para evitar sobrescribirlo
	if (access(full_path.c_str(), F_OK) == 0)
	{
		response.buildSimpleResponse("409", "Conflict"); // 409 Conflict si el recurso ya existe
		return (response);
	}

	// Abrimos un archivo para escribir en el
	std::ofstream new_file(full_path.c_str());
	if (!new_file.is_open())
	{
		// Si no se pudo crear el archivo (ej. por permisos)
		response.buildSimpleResponse("500", "Internal Server Error");
		return (response);
	}

	// Escribimos el cuerpo de la peticion en el archivo
	new_file << request.getBody();
	new_file.close();

	response.buildSimpleResponse("201", "Created");
	return (response);
}

Response MethodHandler::_handleDelete(const Request& request, const Config& config)
{
	std::string full_path = config.getRootDirectory() + request.getPath();
	Response response;

	if (access(full_path.c_str(), F_OK) == -1)
	{
		// Si el archivo no existe, no se puede borrar. Devolvemos 404
		response.buildSimpleResponse("404", "Not Found");
	}
	else if (remove(full_path.c_str()) == 0) // Intentamos borrarlo
	{
		response.setStatusCode("204", "No Content");
	}
	else
	{
		// Si `remove` falla por otra razón (ej. permisos)
		response.buildSimpleResponse("500", "Internal Server Error");
	}
	return (response);
}

// --- Funciones de ayuda ---

std::string MethodHandler::_readFile(const std::string& path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return ("");
	std::stringstream buffer;
	buffer << file.rdbuf();
	return (buffer.str());
}

// Funcion importante para saber que Content-Type es
std::string MethodHandler::_getMimeType(const std::string& path)
{
	std::map<std::string, std::string> mime_types;
	mime_types[".html"] = "text/html";
	mime_types[".css"] = "text/css";
	mime_types[".js"] = "application/javascript";
	mime_types[".jpg"] = "image/jpg";
	mime_types[".jpeg"] = "image/jpeg";
	mime_types[".png"] = "image/png";
	mime_types[".gif"] = "image/gif";
	mime_types[".pdf"] = "application/pdf";

	size_t dot_pos = path.rfind('.');
	if (dot_pos != std::string::npos)
	{
		std::string ext = path.substr(dot_pos);
		if (mime_types.count(ext))
			return (mime_types[ext]);
	}
	// Es el tipo por defecto para descargas
	return ("application/octet-stream");
}

