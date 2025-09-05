#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>
#include <sys/socket.h>

enum ClientState
{
	READING,
	WRITING,
	FINISHED
};

class Client
{
	private:
		int		_socket_fd;
		std::string	_request_buffer;
		std::string	_response_buffer;
		ClientState	_state;

	public:
		Client(int fd);
		~Client();

		// --- Getters ---
		int		getSocketFd() const;
		ClientState	getState() const;
		std::string&	getResponseBuffer();
		std::string&	getRequestBuffer();

		// --- Setters y metodos de accion ---
		void		setState(ClientState new_state);
		void		appendToRequestBuffer(const char* data, size_t len);
		void		setResponseBuffer(const std::string& response);

};

#endif // CLIENT_HPP
