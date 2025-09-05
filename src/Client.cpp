#include "../includes/Client.hpp"

Client::Client(int fd) : _socket_fd(fd), _state(READING)
{
	std::cout << "Default Client constructor called" << std::endl;
}

Client::~Client()
{
	std::cout << "Default Client destructor called" << std::endl;
}

// --- Implementacion de Getters ---

int Client::getSocketFd() const
{
	return (_socket_fd);
}

ClientState Client::getState() const
{
	return (_state);
}

std::string& Client::getResponseBuffer()
{
	return (_response_buffer);
}

std::string& Client::getRequestBuffer()
{
	return (_request_buffer);
}

// --- Implementacion de Setters y Metodos ---

void	Client::setState(ClientState new_state)
{
	_state = new_state;
}

void	Client::appendToRequestBuffer(const char* data, size_t len)
{
	_request_buffer.append(data, len);
}

void	Client::setResponseBuffer(const std::string& response)
{
	_response_buffer = response;
}

