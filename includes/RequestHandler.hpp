#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"
#include <fstream>
#include <unistd.h>

class RequestHandler
{
	private:
		static Response _handleGet(const Request& request, const Config& config);
		static Response _handlePost(const Request& request, const Config& config);
		static Response _handleDelete(const Request& request, const Config& config);

		static std::string _readFile(const std::string& path);

	public:
		static Response handle(const Request& request, const Config& config);
		
};

#endif // REQUESTHANDLER_HPP
