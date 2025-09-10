#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>
#include <sys/socket.h>

enum RequestState
{
	READING_HEADERS,
	READING_BODY,
	READING_READY,
	READING_SENT
};

class Client
{
	private:
		int		_socket_fd;
		std::string	_request_buffer;
		std::string	_response_buffer;
		
		Request		_request;
		RequestState	_request_state;

	public:
		Client(int fd);
		~Client();

		// --- Getters ---
		int		getSocketFd() const;
		std::string&	getResponseBuffer();
		std::string&	getRequestBuffer();
		Request&	getRequest();
		RequestState	getState() const;

		// --- Setters y metodos de accion ---
		void		setState(ClientState new_state);
		void		appendToRequestBuffer(const char* data, size_t len);
		void		setResponseBuffer(const std::string& response);

};

#endif // CLIENT_HPP
