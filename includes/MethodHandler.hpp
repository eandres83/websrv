#ifndef METHODHANDLER_HPP
#define METHODHANDLER_HPP

#include "Response.hpp"
#include "Request.hpp"
#include "Config.hpp"
#include "RequestHandler.hpp"
#include <fstream>
#include <unistd.h>

class MethodHandler
{
	private:
		static Response _handleGet(const Request& request, const Config& config);
		static Response _handlePost(const Request& request, const Config& config);
		static Response _handleDelete(const Request& request, const Config& config);

		static std::string  _readFile(const std::string& path);
		static std::string  _getMimeType(const std::string& path);

	public:
		// metodo publico que gestiona todo
		static Response handle(const Request& request, const Config& config);
};

#endif // METHODHANDLER_HPP
