#ifndef METHODHANDLER_HPP
#define METHODHANDLER_HPP

#include "Response.hpp"
#include "Request.hpp"
#include "Config.hpp"
#include "RequestHandler.hpp"
#include <fstream>
#include <unistd.h>
#include <vector>

class MethodHandler
{
	private:
		static Response _handleGet(Client& client);
		static Response _handlePost(Client& client, const LocationConfig* location);
		static Response _handleDelete(Client& client);

		static std::string  _readFile(const std::string& path);
		static std::string  _getMimeType(const std::string& path);
		static std::string  _parseMultipartBody(const std::string& body, const std::string& boundary);

	public:
		// metodo publico que gestiona todo
		static Response handle(Client& client, const LocationConfig* location);
};

#endif // METHODHANDLER_HPP
