#include "../includes/RequestHandler.hpp"
#include "../includes/MethodHandler.hpp"
#include "../includes/DirectoryHandler.hpp"
#include <sys/stat.h>

Response RequestHandler::handle(const Request& request, const Config& config)
{
	std::string full_path = config.getRootDirectory() + request.getPath();

	// --- LÍNEAS DE DEBUG ---
	std::cout << "Ruta completa recibida: [" << full_path << "]" << std::endl;
	std::cout << "¿Es un directorio? " << (_isDirectory(full_path) ? "Sí" : "No") << std::endl;
	// --- FIN DE LÍNEAS DE DEBUG ---

	// Si es un directorio y tengo permisos en la config, gestiono autoindex
	if (_isDirectory(full_path) && config.getAutoindex())
		return (DirectoryHandler::handle(request, config));
	else
		return (MethodHandler::handle(request, config));
}

bool	RequestHandler::_isDirectory(const std::string& path)
{
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return (false);
	return (S_ISDIR(statbuf.st_mode));
}

