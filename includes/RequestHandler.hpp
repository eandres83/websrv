#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"
#include <fstream>
#include <unistd.h>
#include <dirent.h>
#include <vector>
#include <sys/stat.h>

class RequestHandler
{
	private:
		static bool _isDirectory(const std::string& path);

	public:
		static Response handle(const Request& request, const Config& config);
		
};

#endif // REQUESTHANDLER_HPP
