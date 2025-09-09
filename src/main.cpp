#include "../includes/Server.hpp"
#include "../includes/Config.hpp"

int main(int argc, char **argv)
{
	(void)argc;

	Config config;
	config.parse(argv[1]);
//	char *txt;

//	if (argc == 1 && !config.parse(argv[1]));
//	{
//	        std::cerr << "Error al cargar la configuración." << std::endl;
//	        return 1;
//	}
//
	Server server(config);
	server.run();

	return 0;
}

