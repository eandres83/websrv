#include "../includes/RequestHandler.hpp"
#include "../includes/MethodHandler.hpp"
#include "../includes/DirectoryHandler.hpp"
#include <sys/stat.h>
#include <algorithm>

const LocationConfig* RequestHandler::_findLocationForPath(const std::string& path, const ServerConfig& config)
{
	const LocationConfig* best_match = NULL;
	size_t	longest_match_len = 0;

	for (size_t i = 0; i < config.locations.size(); i++)
	{
		const LocationConfig& loc = config.locations[i];
		if (path.rfind(loc.path, 0) == 0)
		{
			if (loc.path.length() > longest_match_len)
			{
				longest_match_len = loc.path.length();
				best_match = &loc;
			}
		}
	}
	return (best_match);
}

Response RequestHandler::handle(Client& client)
{
	const Request& request = client.getRequest();
	const ServerConfig& config = client.getConfig();
	Response response;

	// 1. Encontrar la location que corresponde a este peticion
	const LocationConfig* location = _findLocationForPath(request.getPath(), config);

	// 1.5. Comprobar si hay una redirección configurada
	if (location && location->return_code != 0)
	{
		// Si la location tiene un return_code, es una redirección.
		std::stringstream ss;
		ss << location->return_code;
		response.setStatusCode(ss.str(), "Moved Permanently");
		response.addHeader("Location", location->return_url);
		response.setBody("<html><body><h1>301 Moved Permanently</h1></body></html>");
		return (response);
	}

	//2. Comprobar si se encontro una location y si el metodo esta permitido
	const std::vector<std::string>* methods_to_check= &config.allowed_methods;

	if (location && !location->allowed_methods.empty())
		methods_to_check = &location->allowed_methods;

	if (!methods_to_check->empty())
	{

		if (std::find(methods_to_check->begin(), methods_to_check->end(), request.getMethod()) == methods_to_check->end())
		{
			response.buildErrorResponse(405, config);
			return (response);
		}
	}

	// 3. Si el metodo esta permitido, continuamos con la logica normal
	std::string full_path = config.root_directory + request.getPath();

	// Si es un directorio y tengo permisos en la config, gestiono autoindex
	if (_isDirectory(full_path))
		return (DirectoryHandler::handle(client, location));
	else
		return (MethodHandler::handle(client));
}

bool	RequestHandler::_isDirectory(const std::string& path)
{
	struct stat statbuf;
	if (stat(path.c_str(), &statbuf) != 0)
		return (false);
	return (S_ISDIR(statbuf.st_mode));
}

