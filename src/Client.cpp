#include "../includes/Client.hpp"

Client::Client(int fd, const ServerConfig& config) : _socket_fd(fd), _bytes_sent(0), _state(READING_HEADERS), _config(config)
{
}

Client::~Client()
{
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

size_t Client::getBytesSent() const
{
	return (_bytes_sent);
}

const ServerConfig& Client::getConfig() const
{
	return (_config);
}

Request& Client::getRequest()
{
	return (_request);
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

void	Client::incrementBytesSent(size_t bytes)
{
	_bytes_sent += bytes;
}

