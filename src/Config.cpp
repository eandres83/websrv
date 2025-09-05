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
	_port = 8080;
	_root_directory = "./www";
	_error_page_404 = "./www/404.html";
	std::cout << "Configuración cargada. Usando el puerto: " << _port << std::endl;
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

