#include "../includes/Config.hpp"

Config::Config()
{
}

Config::~Config()
{
}

// Por ahora, solo establece un puerto por defecto para que el servidor arranque
bool Config::parse(const char* filename)
{
	(void)filename;
	ServerConfig defaultConfig;
	defaultConfig.port = 8080;
	defaultConfig.root_directory = "./www";
	defaultConfig.error_pages[404] = "./www/404.html";
	defaultConfig.error_pages[409] = "./www/409.html";
	defaultConfig.enable_reuse_addr = 1;
	defaultConfig.client_max_body_size = 1048576; // 1MB en bytes
	defaultConfig.allowed_methods.push_back("GET");

	// --- Test redirection ---
	LocationConfig redirect_location;
	redirect_location.path = "/pagina-antigua";
	redirect_location.root_directory = "./www";
	redirect_location.return_code = 301;
	redirect_location.return_url = "https://www.youtube.com";

	// --- Hardcodeado las locations para probarlo ---
	LocationConfig root_location;
	root_location.path = "/";
	root_location.root_directory = "./www";
	root_location.allowed_methods.push_back("GET");
	root_location.autoindex = true;
	root_location.return_code = 0;
	root_location.index_files.push_back("index.html");

	LocationConfig uploads_location;
	uploads_location.path = "/uploads";
	uploads_location.root_directory = "./www";
	uploads_location.autoindex = true;
	uploads_location.allowed_methods.push_back("GET");
	uploads_location.allowed_methods.push_back("POST");
	uploads_location.index_files.push_back("index.html");
	uploads_location.index_files.push_back("index2.html");

	// Tester
	LocationConfig test;
	test.path = "/directory/";
	test.root_directory = "./YoupiBanane";
	test.allowed_methods.push_back("GET");
	test.index_files.push_back("youpi.bad_extension");
	test.autoindex = true;

	LocationConfig test_location;
	test_location.path = "/static";
	test_location.root_directory = "./www_static";
	test_location.allowed_methods.push_back("GET");
	test_location.autoindex = true;

	defaultConfig.locations.push_back(root_location);
	defaultConfig.locations.push_back(uploads_location);
	defaultConfig.locations.push_back(redirect_location);
	defaultConfig.locations.push_back(test_location);
//	defaultConfig.locations.push_back(test);

	_server_configs.push_back(defaultConfig);

	return (true);
}

const std::vector<ServerConfig>& Config::getServerConfigs() const
{
	return (_server_configs);
}

//int Config::getPort() const
//{
//	return (_port);
//}
//
//const std::string& Config::getRootDirectory() const
//{
//	return (_root_directory);
//}
//
//const std::string& Config::getErrorPage404() const
//{
//	return (_error_page_404);
//}
//
//bool Config::getAutoindex() const
//{
//	return (_autoindex);
//}
//
//int	Config::getReuseAddr() const
//{
//	return (_enable_reuse_addr);
//}
