#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>
#include <sys/socket.h>
#include "Config.hpp"
#include "Request.hpp"

enum ClientState
{
	READING_HEADERS,
	READING_BODY,
	REQUEST_READY,
	WRITING  // El servidor esta escribiendo datos al cliente
};

class Client
{
	private:
		int		_socket_fd;
		std::string	_request_buffer;
		std::string	_response_buffer;
		size_t		_bytes_sent;
		ClientState	_state;	
		Request		_request;
		const ServerConfig& _config;

	public:
		Client(int fd, const ServerConfig& config);
		~Client();

		// --- Getters ---
		int		getSocketFd() const;
		std::string&	getResponseBuffer();
		std::string&	getRequestBuffer();
		Request&	getRequest();
		ClientState	getState() const;
		const ServerConfig& getConfig() const;
		size_t		getBytesSent() const;

		// --- Setters y metodos de accion ---
		void		setState(ClientState new_state);
		void		appendToRequestBuffer(const char* data, size_t len);
		void		setResponseBuffer(const std::string& response);
		void		incrementBytesSent(size_t bytes);

};

#endif // CLIENT_HPP
