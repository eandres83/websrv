#include "../includes/MethodHandler.hpp"

// Funcion principal que gestiona todo los metodos
Response MethodHandler::handle(Client& client)
{
	if (client.getRequest().getMethod() == "GET")
		return (_handleGet(client));
	else if (client.getRequest().getMethod() == "POST")
		return (_handlePost(client));
	else if (client.getRequest().getMethod() == "DELETE")
		return (_handleDelete(client));

	Response response;
	response.buildErrorResponse(501, client.getConfig());
	return (response);
}

// --- Implementacion de los metodos --- 

Response MethodHandler::_handleGet(Client& client)
{
	const Request& request = client.getRequest();
	const ServerConfig& config = client.getConfig();

	std::string full_path = request.getFullPath();

	Response response;

	if (access(full_path.c_str(), F_OK) == -1) // Si el archivo no existe
	{
		response.buildErrorResponse(404, config);
	}
	else // Si el archivo existe
	{
		response.buildCustomResponse("200", "OK", _readFile(full_path));
		response.addHeader("Content-Type", _getMimeType(full_path));
	}
	return (response);
}

Response MethodHandler::_handlePost(Client& client)
{
	// Construimos la ruta donde se guardara el archivo
	// Ej: si la petición es a /nuevo, la ruta sera ./www/nuevo
	// Porque el rootDirecotry es ./www que se define en el archivo de confg
	const Request& request = client.getRequest();
	const ServerConfig& config = client.getConfig();

	std::string full_path = request.getFullPath();
	Response response;

	// Comprobamos si el recurso ya existe para evitar sobrescribirlo
	if (access(full_path.c_str(), F_OK) == 0)
	{
		response.buildErrorResponse(409, config); // 409 Conflict si el recurso ya existe
		return (response);
	}

	// Abrimos un archivo para escribir en el
	std::ofstream new_file(full_path.c_str());
	if (!new_file.is_open())
	{
		// Si no se pudo crear el archivo (ej. por permisos)
		response.buildErrorResponse(500, config);
		return (response);
	}

	// Escribimos el cuerpo de la peticion en el archivo
	new_file << request.getBody();
	new_file.close();

	response.buildErrorResponse(201, config);
	return (response);
}

Response MethodHandler::_handleDelete(Client& client)
{
	const Request& request = client.getRequest();
	const ServerConfig& config = client.getConfig();

	std::string full_path = request.getFullPath();
	Response response;

	if (access(full_path.c_str(), F_OK) == -1)
	{
		// Si el archivo no existe, no se puede borrar. Devolvemos 404
		response.buildErrorResponse(404, config);
	}
	else if (remove(full_path.c_str()) == 0) // Intentamos borrarlo
	{
		response.buildErrorResponse(204, config);
	}
	else
	{
		// Si `remove` falla por otra razón (ej. permisos)
		response.buildErrorResponse(500, config);
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

