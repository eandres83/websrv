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
		static Response _handleGet(Client& client);
		static Response _handlePost(Client& client);
		static Response _handleDelete(Client& client);

		static std::string  _readFile(const std::string& path);
		
	public:
		static std::string  getMimeType(const std::string& path);
		// metodo publico que gestiona todo
		static Response handle(Client& client);
};

#endif // METHODHANDLER_HPP
