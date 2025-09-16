#include "../includes/Server.hpp"
#include "../includes/Config.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Uso: " << argv[0] << " <archivo_de_configuracion>" << std::endl;
		return (1);
	}

	Config config;
	if (!config.parse(argv[1]))
	{
		std::cerr << "Error al cargar la configuracion." << std::endl;
		return (1);
	}

	try
	{
		Server server(config);
		server.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
	

	return (0);
}
