#ifndef SERVER_HPP
#define SERVER_HPP

#include <unistd.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "Client.hpp"
#include "Config.hpp"
#include "Response.hpp"

#define MAX_FDS 200

class Server
{
	private:
		const Config& _config;
		std::map<int, Client> _clients;
		std::vector<int> _listenSockets;
		std::map<int, ServerConfig> _listener_configs;

		void acceptNewConnection(int listener_fd, int epoll_fd);
		void handleClientRequest(int client_fd, int epoll_fd);
		void handleClientResponse(int client_fd, int epoll_fd);
		void closeClientConnection(int client_fd, int epoll_fd);

	public:
		Server(const Config& config);
		~Server();
		void run();
};

#endif // SERVER_HPP
