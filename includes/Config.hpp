#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <string>
#include <map>
#include <vector>

// Para implementar la gestion de multiples servidores
// en el parseo por cada bloque de server tienes que crear un objeto ServerConfig
// y rellenarlo y anadirlo al vector _server_configs

struct LocationConfig
{
	std::string path; // ej: "/" o "/images"
	std::vector<std::string> allowed_methods; // ej: ["GET", "POST"]
	bool	autoindex;
	std::vector<std::string> index_files;
	int		return_code;
	std::string	return_url;
	std::string	root_directory;

	LocationConfig(): autoindex(false), return_code(0) {}
};

struct ServerConfig
{
	int 		port;
	std::string 	root_directory;
	int		enable_reuse_addr;
	bool		autoindex;
	long long	client_max_body_size;

	std::vector<std::string> allowed_methods;
	std::map<int, std::string> error_pages;
	std::vector<LocationConfig> locations;
	
	ServerConfig(): autoindex(false), client_max_body_size(1048576) {}
};

class Config
{
	private:
		std::vector<ServerConfig> _server_configs;
		std::map<int, ServerConfig> _listener_configs;

	public:
		Config();
		~Config();

		bool					parse(const char* filename);
		const std::vector<ServerConfig>&	getServerConfigs() const;
};

#endif // CONFIG_HPP
