#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>		//	Standard library headers for I/O, strings, maps, and vectors.
#include <string>
#include <map>
#include <vector>
#include <sstream>		//	For parsing purposes
#include <fstream>		//	Also for parsing

//	Forward declaration of a struct or class allows you to refer to it before it's fully defined, which can be useful for circular dependencies.
//	In this case, it's a struct to hold the configuration for a specific `location` block.
//	Each `location` block in the config file will correspond to an instance of this struct.
struct	LocationConfig
{
	std::string 				path; 				//	This is the URI (Uniform Resource Identifier) path that this location block applies to. E.g., "/" or "/images". 
	std::vector<std::string>	allowed_methods; 	//	A list of allowed HTTP methods (called 'verbs' here) for this path. ["GET", "POST", "PUT", "DELETE"]. 
	bool						autoindex; 			//	'true' if autoindex is 'on' for this location, 'false' if 'off'.
	std::vector<std::string>	index_files; 		//	List of default files to look for in a directory. E.g., ["index.html", "index.php"].
	int							return_code; 		//	The HTTP status code for a redirect (e.g., 301 for a permanent redirect).
	std::string 				return_url;			//	The URL to redirect to if a return code is specified.
	std::string					root_directory;		//	A new root directory for this location, which can override the server's root.
	std::string 				upload_path;		//	A directory for handling file uploads via POST requests, specific to this location.

	LocationConfig(): autoindex(false), return_code(0) {}	//	A constructor to initialize the default values.
};

//	Another forward declaration of a struct, in this case a struct to hold the configuration for an entire `server` block.
//	Each `server` block in the config file will be an instance of this struct.
//	IMPORTANT: `Server` settings are "by default" and may be overriden by specific `location` settings.
struct	ServerConfig
{
	int							port;					//	The port number on which this server will listen.
	std::string					root_directory;			//	The base directory for this virtual host.
	int							enable_reuse_addr;		//	A flag for socket options, likely to allow the port to be reused quickly after a program exits.
	bool						autoindex;				//	The default autoindex setting for the entire server.
	long long					client_max_body_size;	//	The maximum size of a client's request body (e.g., for file uploads). The default is 1 MB.
	std::vector<std::string>	allowed_methods;		//	Allowed methods at the server level, before any location-specific rules. ["GET", "POST", "PUT", "DELETE"].
	std::map<int, std::string>	error_pages;			//	A map to store custom error pages (e.g., key 404 maps to value "./www/404.html").
	std::vector<LocationConfig>	locations;				//	A vector to store all the `LocationConfig` objects for this server.	

	ServerConfig(): autoindex(false), client_max_body_size(1048576) {}	//	A constructor to set default values for autoindex and client_max_body_size.
};

//	The main class that will manage all the server configurations.
class	Config
{
	private:
		std::vector<ServerConfig>	_server_configs;	//	A vector to store all the parsed server configurations.
		std::map<int, ServerConfig>	_listener_configs;	//	A map to quickly access a server config by its listening port. This is a common way to manage multiple servers.

		bool	ParseServerBlock(const std::string& content, size_t& index);										//	Server-specific parsing functions.
		bool	ParseListenDirective(const std::string& content, size_t& index, ServerConfig& server);
		bool	ParseClientMaxBodySizeDirective(const std::string& content, size_t& index, ServerConfig& server);

		
	public:
		Config(); 										// Default constructor.
		~Config(); 										// Destructor.
		
		bool	parse(const char* filename);							//	Method to start the parsing process.
		const std::vector<ServerConfig>&	getServerConfigs() const;	//	Getter method to access (read-only) server configurations.
};

template <typename ConfigT>																							//	Shared parsing functions.
bool	ParseRootDirectiveT(const std::string& content, size_t& index, ConfigT& config_struct);
template <typename ConfigT>
bool	ParseAutoindexDirectiveT(const std::string& content, size_t& index, ConfigT& config_struct);
template <typename ConfigT>
bool	ParseErrorPageDirectiveT(const std::string& content, size_t& index, ConfigT& config_struct);

#include "ParserTemplates.tpp"

#endif
