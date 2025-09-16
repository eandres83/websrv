#include "../includes/Server.hpp"
#include "../includes/Request.hpp"
#include "../includes/RequestHandler.hpp"

Server::Server(const Config& config): _config(config)
{
	// Obtenemos el vector de todas las configs del server
	const std::vector<ServerConfig>& server_configs = _config.getServerConfigs();

	for (size_t i = 0; i < server_configs.size(); i++)
	{
		const ServerConfig& sc = server_configs[i];
		int port = sc.port;
		int server_fd;
		struct sockaddr_in address;

		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		{
			throw std::runtime_error("Error fatal: socket() fallo");
		}
	
		// Permitir reutilizar la dirección/puerto inmediatamente
		int enable_reuse_addr = sc.enable_reuse_addr;
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable_reuse_addr, sizeof(enable_reuse_addr)) < 0)
		{
			close(server_fd);
			throw std::runtime_error("Error fatal: setsockopt() fallo");
		}
	
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);
	
		if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
		{
			close(server_fd);
			throw std::runtime_error("Error fatal: bind() fallo");
		}
	
		if (listen(server_fd, 10) < 0)
		{
			close(server_fd);
			throw std::runtime_error("Error fatal: listen() fallo");
		}
		// Guardamos el nuevo socket de escucha
		_listenSockets.push_back(server_fd);
		_listener_configs[server_fd] = sc; // Asociamos el fd con su config por si tenemos varios servidores
		std::cout << "Servidor escuchando en el puerto " << port << std::endl;
	}
}

Server::~Server()
{

}

void Server::run()
{
	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
		throw std::runtime_error("Error fatal: epoll_create");

	const int MAX_EVENTS = 64;
	struct epoll_event events[MAX_EVENTS];

	// Registra los sockets de escucha iniciales en la instancia de epoll
	for (size_t i = 0; i < _listenSockets.size(); ++i)
	{
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = _listenSockets[i];
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _listenSockets[i], &event) == -1)
			throw std::runtime_error("Error fatal: epoll_ctl");
	}

	// Bucle principal de eventos del servidor
	while (true)
	{
		// epoll_wait bloquea hasta que ocurra un evento en los sockets monitorizados
		int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (nfds == -1)
			throw std::runtime_error("Error fatal: epoll_wait");
		for (int i = 0; i < nfds; ++i)
		{
			int current_fd = events[i].data.fd;
			bool is_listener = false;
			for(size_t j = 0; j < _listenSockets.size(); ++j)
			{
				if (current_fd == _listenSockets[j])
				{
					is_listener = true;
					break;
				}
			}
			// Distingue entre una nueva conexion y datos de un cliente existente
			if (is_listener)
				acceptNewConnection(current_fd, epoll_fd);
			else
			{
				std::map<int, Client>::iterator it = _clients.find(current_fd);
				if (it == _clients.end())
					continue; // Cliente no encontrado, seguimos
				Client& client = it->second;

				// La logica ahora depende directamente del estado del cliente
				if (client.getState() == WRITING && (events[i].events & EPOLLOUT))
					handleClientResponse(current_fd, epoll_fd);
				else if (client.getState() == READING_HEADERS && (events[i].events & EPOLLIN))
					handleClientRequest(current_fd, epoll_fd);
			}
		}
	}
}

void Server::acceptNewConnection(int listener_fd, int epoll_fd)
{
	int client_socket = accept(listener_fd, NULL, NULL);
	if (client_socket == -1)
	{
		std::cerr << "accept" << std::endl;
		return;
	}
	if (fcntl(client_socket, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "Error: fcntl" << std::endl;
		return;
	}

	// Buscamos la configuracion correcta usando el listener_fd
	const ServerConfig& client_config = _listener_configs.at(listener_fd);
	// Creamos el cliente, pasandole su configuracion especifica
	_clients.insert(std::make_pair(client_socket, Client(client_socket, client_config)));
	std::cout << "Nuevo cliente conectado en puerto " << client_config.port << " con fd: " << client_socket << std::endl;

	struct epoll_event client_event;
	client_event.events = EPOLLIN | EPOLLET; // Edge-Triggered para eficiencia.
	client_event.data.fd = client_socket;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &client_event) == -1)
	{
		std::cerr << "epoll_ctl: add client_socket" << std::endl;
		close(client_socket);
		_clients.erase(client_socket);
	}
}

// websrv/src/Server.cpp

void Server::handleClientRequest(int client_fd, int epoll_fd)
{
	Client& client = _clients.at(client_fd);
	const ServerConfig& config = client.getConfig();
	char buffer[4096];
	ssize_t bytes_read;

	// --- 1. Leer del socket en un bucle ---
	// Con EPOLLET, leemos hasta que el buffer del kernel esté vacío.
	while (true)
	{
		bytes_read = read(client_fd, buffer, sizeof(buffer));
		if (bytes_read > 0)
		{
			client.appendToRequestBuffer(buffer, bytes_read);
		}
		else if (bytes_read == 0)
		{
			// El cliente cerró la conexión.
			closeClientConnection(client_fd, epoll_fd);
			return;
		}
		else // bytes_read == -1
		{
			// Un -1 en un socket no bloqueante significa "no hay más datos por ahora".
			// Rompemos el bucle para procesar lo que hemos leído.
			break;
		}
	}

	// --- 2. Comprobar si el tamaño excede el límite ---
	if (client.getRequestBuffer().length() > static_cast<size_t>(config.client_max_body_size))
	{
		Response response;
		response.buildErrorResponse(413, config); // 413 Payload Too Large
		client.setResponseBuffer(response.toString());
		client.setState(WRITING);

		struct epoll_event ev;
		ev.events = EPOLLOUT | EPOLLET;
		ev.data.fd = client_fd;
		epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev);
		return;
	}

	// --- 3. Máquina de estados para el parseo ---
	if (client.getState() == READING_HEADERS)
	{
		size_t end_of_headers = client.getRequestBuffer().find("\r\n\r\n");
		if (end_of_headers != std::string::npos)
		{
			std::string header_part = client.getRequestBuffer().substr(0, end_of_headers);
			client.getRequest().parse(header_part);

			std::string body_part = client.getRequestBuffer().substr(end_of_headers + 4);
			client.getRequest().setBody(body_part);

			std::string cl_value = client.getRequest().getHeaderValue("Content-Length");
			if (!cl_value.empty())
			{
				client.setState(READING_BODY);
				// No hacemos 'return', continuamos al siguiente 'if' por si el body ya está completo.
			}
			else
			{
				client.setState(REQUEST_READY);
			}
		}
	}

	if (client.getState() == READING_BODY)
	{
		size_t headers_len = client.getRequestBuffer().find("\r\n\r\n") + 4;
		client.getRequest().setBody(client.getRequestBuffer().substr(headers_len));
		
		std::string cl_value = client.getRequest().getHeaderValue("Content-Length");
		char* end = NULL;
		unsigned long len = std::strtoul(cl_value.c_str(), &end, 10);
		
		if (client.getRequest().getBody().length() >= len)
		{
			// Nos aseguramos de que el body no contenga datos de más.
			client.getRequest().setBody(client.getRequest().getBody().substr(0, len));
			client.setState(REQUEST_READY);
		}
	}

	if (client.getState() == REQUEST_READY)
	{
		Response response = RequestHandler::handle(client);
		client.setResponseBuffer(response.toString());
		client.setState(WRITING);

		struct epoll_event client_event;
		client_event.events = EPOLLOUT | EPOLLET;
		client_event.data.fd = client_fd;
		epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &client_event);
	}
}

void Server::handleClientResponse(int client_fd, int epoll_fd)
{
	std::map<int, Client>::iterator it = _clients.find(client_fd);
	if (it == _clients.end())
		return;
	Client& client = it->second;

	const std::string& response = client.getResponseBuffer();
	size_t total_size = response.length();
	size_t bytes_sent = client.getBytesSent();
	size_t remaining = total_size - bytes_sent;

	if (remaining == 0) // No debería pasar, pero por si acaso
	{
		closeClientConnection(client_fd, epoll_fd);
		return;
	}

	// Intentamos enviar los datos restantes
	ssize_t sent_now = write(client_fd, response.c_str() + bytes_sent, remaining);

	if (sent_now == -1)
	{
		std::cerr << "Error al escribir en el socket del cliente " << client_fd << ", cerrando conextion." << std::endl;
		closeClientConnection(client_fd, epoll_fd);
		return;
	}

	client.incrementBytesSent(sent_now);

	// Comprobamos si ya hemos terminado de enviar todo
	if (client.getBytesSent() == total_size)
	{
		std::cout << "Respuesta enviada completamente al cliente " << client_fd << std::endl;
		closeClientConnection(client_fd, epoll_fd);
	}
	// Si no, NO hacemos nada. El servidor esperará el próximo evento EPOLLOUT
	// para volver a llamar a esta función y enviar el siguiente trozo.
}

void Server::closeClientConnection(int client_fd, int epoll_fd)
{
	// Elimina el cliente de epoll, cierra el socket y lo borra del mapa
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
	close(client_fd);
	_clients.erase(client_fd);
	std::cout << "Cliente " << client_fd << " desconectado." << std::endl;
}

