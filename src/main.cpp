#include "../includes/Server.hpp"
#include "../includes/Config.hpp"

int main(int argc, char **argv)
{
	Config config;
	if (!config.parse(argc > 1 ? argv[1] : "default.conf"))
	{
	        std::cerr << "Error al cargar la configuración." << std::endl;
	        return 1;
	}

	Server server(config);
	server.run();

	return 0;
}

