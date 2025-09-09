#include "../includes/DirectoryHandler.hpp"

Response DirectoryHandler::handle(const Request& request, const Config& config)
{
	std::string path = request.getPath();
	std::string full_path = config.getRootDirectory() + path;
	Response response;

	if (!_isDirectory(full_path))
	{
		response.buildSimpleResponse("500", "Internal Server Error");
		return (response);
	}

	// Si index.html esta en el directorio tengo que habrirlo, si no autoindex
	std::string index_path = full_path + "/index.html";

	if (access(index_path.c_str(), F_OK) == 0) // Si index.html existe
	{
		response.buildCustomResponse("200", "OK", _readFile(index_path));
		response.addHeader("Content-Type", "text/html");
	}
	else // Si index.html no existe
	{
		// El objeto config llama a la funcion getAutoindex() que comprueba si esta activo en el archivo de configuracion
		if (config.getAutoindex())
			return (_generateAutoindexPage(path, full_path));
		else
			response.buildSimpleResponse("403", "Forbidden");
	}
	return (response);
}

// --- Implementacion de los metodos privados ---

Response DirectoryHandler::_generateAutoindexPage(const std::string& path, const std::string& directory)
{
	Response response;
	std::stringstream body_stream;

	body_stream << "<html><head><title>Index of " << path << "</title></head>";
	body_stream << "<body><h1>Index of " << path << "</h1><hr><ul>";

	DIR* dir = opendir(directory.c_str()); // La funcion opendir funcioan con un tipo de dato DIR
	if (dir != NULL)
	{
		struct dirent* entry;
		while ((entry = readdir(dir)) != NULL) // Lo mismo, readdir funciona con una estructura dirent
		{
			std::string name = entry->d_name;
			std::string full_path = path;
			if (full_path[full_path.length() - 1] != '/') // Compruebo si acaba con una '/' ns si es 100% necesario
				full_path += '/';
			body_stream << "<li><a href=\"" << full_path << name << "\">" << name << "</a></li>";
		}
		closedir(dir);
	}
	body_stream << "</ul><hr></body></html>";

	response.buildCustomResponse("200", "OK", body_stream.str()); // Crea un response con el body que se ha ido creando
	return (response);
}

// Funcion para saber si es un directorio
bool	DirectoryHandler::_isDirectory(const std::string& path)
{
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return (false);
	return (S_ISDIR(statbuf.st_mode)); // Devuelve true si es un directorio
}

// Funciona para leer un archivo
std::string DirectoryHandler::_readFile(const std::string& path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		return ("");
	std::stringstream buffer;
	buffer << file.rdbuf();
	return (buffer.str());
}

