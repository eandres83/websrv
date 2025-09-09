#include "../includes/Config.hpp"

Config::Config() : _port(0)
{

}

Config::~Config()
{

}

// Por ahora, solo establece un puerto por defecto para que el servidor arranque
bool Config::parse(const char* filename)
{
	(void)filename;
	_port = 8081;
	_root_directory = "./www";
	_error_page_404 = "./www/404.html";
	_autoindex = true;
	_enable_reuse_addr = 1;

	std::cout << "Configuración cargada. Puerto: " << _port << ", Root: " << _root_directory << std::endl;
	return (true);
}

int Config::getPort() const
{
	return (_port);
}

const std::string& Config::getRootDirectory() const
{
	return (_root_directory);
}

const std::string& Config::getErrorPage404() const
{
	return (_error_page_404);
}

bool Config::getAutoindex() const
{
	return (_autoindex);
}

int	Config::getReuseAddr() const
{
	return (_enable_reuse_addr);
}