#include "../includes/Server.hpp"
#include "../includes/Request.hpp"
#include "../includes/RequestHandler.hpp"

Server::Server(const Config& config): _config(config)
{
	// Esta seccion inicializa y vincula los sockets de escucha del servidor
	int port = _config.getPort();
	int server_fd;
	struct sockaddr_in address;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 10) < 0)
	{
		perror("listen failed");
		exit(EXIT_FAILURE);
	}
	_listenSockets.push_back(server_fd);
	std::cout << "Servidor escuchando en el puerto " << port << std::endl;
}

Server::~Server()
{

}

void Server::run()
{
	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}

	const int MAX_EVENTS = 64;
	struct epoll_event events[MAX_EVENTS];

	// Registra los sockets de escucha iniciales en la instancia de epoll
	for (size_t i = 0; i < _listenSockets.size(); ++i)
	{
		struct epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = _listenSockets[i];
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _listenSockets[i], &event) == -1)
		{
			perror("epoll_ctl: listen_socket");
			exit(EXIT_FAILURE);
		}
	}

	// Bucle principal de eventos del servidor
	while (true)
	{
		// epoll_wait bloquea hasta que ocurra un evento en los sockets monitorizados
		int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (nfds == -1)
		{
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}

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
				if (events[i].events & EPOLLIN) // Hay datos para leer del cliente
					handleClientRequest(current_fd, epoll_fd);
				else if (events[i].events & EPOLLOUT) // Se puede escribir al cliente
					handleClientResponse(current_fd, epoll_fd);
			}
		}
	}
}

void Server::acceptNewConnection(int listener_fd, int epoll_fd)
{
	int client_socket = accept(listener_fd, NULL, NULL);
	if (client_socket == -1)
	{
		perror("accept");
		return;
	}
	fcntl(client_socket, F_SETFL, O_NONBLOCK);

	// Registra el nuevo cliente en el mapa y lo anade a epoll para leer su peticion
	_clients.insert(std::make_pair(client_socket, Client(client_socket)));
	std::cout << "Nuevo cliente conectado con fd: " << client_socket << std::endl;

	struct epoll_event client_event;
	client_event.events = EPOLLIN | EPOLLET; // Edge-Triggered para eficiencia.
	client_event.data.fd = client_socket;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &client_event) == -1)
	{
		perror("epoll_ctl: add client_socket");
		close(client_socket);
		_clients.erase(client_socket);
	}
}

void Server::handleClientRequest(int client_fd, int epoll_fd)
{
	char buffer[1024];
	ssize_t bytes_read;

	// Bucle para leer todos los datos disponibles
	// Esto se hace porque si el cliente envia 2000 bytes solo se almacenan 1024 en bytes_read
	while (true)
	{
		bytes_read = read(client_fd, buffer, sizeof(buffer));
		if (bytes_read == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break ;
			closeClientConnection(client_fd, epoll_fd);
			return ;
		}
		if (bytes_read == 0)
		{
			closeClientConnection(client_fd, epoll_fd);
			return ;
		}
		_clients.at(client_fd).appendToRequestBuffer(buffer, bytes_read);
	}

	// El parser de igchurru ira aqui. Por ahora, lo simulamos
	Request request;
	request.parse(_clients.at(client_fd).getRequestBuffer()); // El parser llenara el objeto request

	// Le pasamos la peticion al handler para que haga todo el trabajo
	Response response = RequestHandler::handle(request, _config);

	// Tomamos la respuesta y la preparamos para el envio.
	_clients.at(client_fd).setResponseBuffer(response.toString());
	_clients.at(client_fd).setState(WRITING);

	// Modificamos epoll para vigilar eventos de escritura (EPOLLOUT)
	struct epoll_event client_event;
	client_event.events = EPOLLOUT | EPOLLET;
	client_event.data.fd = client_fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &client_event);
}

void Server::handleClientResponse(int client_fd, int epoll_fd)
{
	std::string& response = _clients.at(client_fd).getResponseBuffer();
	ssize_t bytes_sent = write(client_fd, response.c_str(), response.length());

	if (bytes_sent == -1)
	{
		closeClientConnection(client_fd, epoll_fd);
		return;
	}

	// Una vez enviada la respuesta, se cierra la conexion (comportamiento simple)
	std::cout << "Respuesta enviada al cliente " << client_fd << std::endl;
	closeClientConnection(client_fd, epoll_fd);
}

void Server::closeClientConnection(int client_fd, int epoll_fd)
{
	// Elimina el cliente de epoll, cierra el socket y lo borra del mapa
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
	close(client_fd);
	_clients.erase(client_fd);
	std::cout << "Cliente " << client_fd << " desconectado." << std::endl;
}

