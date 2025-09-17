#include "Colors.hpp"
#include "Client.hpp"
#include "Response.hpp"
#include "MethodHandler.hpp"
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <cstring>

#ifndef CGI_HPP
#define CGI_HPP

int manageCGI(Client &client, Response &response);

#endif // CGI_HPP
