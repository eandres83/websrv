#include "../includes/CGI.hpp"

// Funcion principal que gestiona todo los metodos
Response MethodHandler::handle(Client& client, const LocationConfig* location)
{
	if (client.getRequest().getMethod() == "GET")
		return (_handleGet(client));
	else if (client.getRequest().getMethod() == "POST")
		return (_handlePost(client, location));
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
		if (request.getIsCGI())
		{
			// LOGICA DEL CGI
			manageCGI(client, response);
		}
		else
		{
			response.buildCustomResponse("200", "OK", _readFile(full_path));
			response.addHeader("Content-Type", getMimeType(full_path));
		}
	}
	return (response);
}

Response MethodHandler::_handlePost(Client& client, const LocationConfig* location)
{
	const Request& request = client.getRequest();
	const ServerConfig& config = client.getConfig();
	std::string full_path = request.getFullPath();
	Response response;

	std::string final_path;
	if (location && !location->upload_path.empty())
	{
		std::string request_path = request.getPath();
		size_t last_slash = request_path.rfind('/');
		std::string filename = (last_slash != std::string::npos) ? request_path.substr(last_slash + 1) : request_path;

		final_path = location->upload_path;
		if (final_path[final_path.length() - 1] != '/')
			final_path += '/';
		final_path += filename;
	}
	else
		final_path = request.getFullPath();

	// 1. Conseguir en boundary de la cabecera 'Content-Type'
	std::string content_type = request.getHeaderValue("Content-Type");
	std::string boundary;
	size_t boundary_pos = content_type.find("boundary=");
	if (boundary_pos != std::string::npos)
		boundary = content_type.substr(boundary_pos + 9);
	else
	{
		// Si no es multipart/form-data, lo tratamos como un cuerpo simple
		if (access(final_path.c_str(), F_OK) == 0)
			return (response.buildErrorResponse(409, config), response);
		std::ofstream new_file(final_path.c_str(), std::ios::binary);
		if (!new_file.is_open())
			return (response.buildErrorResponse(500, config), response);
		new_file << request.getBody();
		new_file.close();
		return (response.buildErrorResponse(201, config), response);
	}

	// 2. Comprobar si el recurso ya existe
	if (access(final_path.c_str(), F_OK) == 0)
		return (response.buildErrorResponse(409, config), response);

	// 3. Parsear el cuerpo para extraer solo el contenido del archivo
	std::string file_content = MethodHandler::_parseMultipartBody(request.getBody(), boundary);
	if (file_content.empty())
		return (response.buildErrorResponse(400, config), response);

	// 4. Escribir el contenido extraido en el archivo
	std::ofstream new_file(final_path.c_str(), std::ios::binary);
	if (!new_file.is_open())
		return (response.buildErrorResponse(500, config), response);

	new_file.write(file_content.c_str(), file_content.length());
	new_file.close();

	return (response.buildErrorResponse(201, config), response);
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

// Extrae el contenido del archivo de un cuerpo multipart/form-data
std::string MethodHandler::_parseMultipartBody(const std::string& body, const std::string& boundary)
{
	std::string boundary_sep = "--" + boundary;
	std::string boundary_end = boundary_sep + "--";

	// 1. Encontrar el inicio del cuerpo, la primera aparicion del separador de boundary
	size_t start_pos = body.find(boundary_sep);
	if (start_pos == std::string::npos)
		return ("");

	// Buscamos el final de la cabecera
	size_t headers_end = body.find("\r\n\r\n", start_pos);
	if (headers_end == std::string::npos)
		return ("");

	// El contenido del archivo empieza justo despues
	size_t content_start = headers_end + 4;

	// 2. Encontrar el final del cuerpo
	size_t content_end = body.find(boundary_sep, content_start);
	if (content_end == std::string::npos)
		return ("");

	// Extraemos el contenido, restamos 2 para quitar el "\r\n" final
	return (body.substr(content_start, content_end - content_start - 2));
}

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
std::string MethodHandler::getMimeType(const std::string& path)
{
	std::map<std::string, std::string> mime_types;
	// Tipos de texto
	mime_types[".html"] = "text/html";
	mime_types[".css"] = "text/css";
	mime_types[".js"] = "application/javascript";
	mime_types[".json"] = "application/json";
	mime_types[".xml"] = "application/xml";
	// Tipos de imagen
	mime_types[".jpg"] = "image/jpg";
	mime_types[".jpeg"] = "image/jpeg";
	mime_types[".png"] = "image/png";
	mime_types[".gif"] = "image/gif";
	mime_types[".svg"] = "image/svg+xml";
	// Tipos de video
	mime_types[".mp4"] = "video/mp4";
	mime_types[".webm"] = "video/webm";
	mime_types[".ogg"] = "video/ogg";
	mime_types[".avi"] = "video/x-msvideo";
	mime_types[".mov"] = "video/quicktime";
	// Other
	mime_types[".pdf"] = "application/pdf";
	mime_types[".php"] = "/usr/bin/php";
	mime_types[".py"] = "/usr/bin/python3";
	mime_types[".sh"] = "/usr/bin/bash";
	mime_types[".zip"] = "application/zip";
	mime_types[".rar"] = "application/x-rar-compressed";

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

