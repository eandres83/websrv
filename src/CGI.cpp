/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nquecedo <nquecedo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/10 18:14:25 by nquecedo          #+#    #+#             */
/*   Updated: 2025/09/22 19:31:42 by nquecedo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CGI.hpp"
#include <fcntl.h>
#include <errno.h>

std::vector<char *> setEnv(Client &client)
{
    const Request &request = client.getRequest();
    const ServerConfig &config = client.getConfig();

    std::vector<std::string> env_vars;
    env_vars.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env_vars.push_back("SERVER_SOFTWARE=webserv/1.0");
    env_vars.push_back("SERVER_NAME=localhost");
    {
        std::ostringstream ss; ss << config.port;
        env_vars.push_back("SERVER_PORT=" + ss.str());
    }
    env_vars.push_back("SERVER_PROTOCOL=" + request.getHttpVersion());
    env_vars.push_back("REQUEST_METHOD=" + request.getMethod());
    env_vars.push_back("QUERY_STRING=" + request.getQueryString());
    env_vars.push_back("PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin");

    // Rutas
    env_vars.push_back("SCRIPT_FILENAME=" + (config.root_directory + request.getPath()));
    env_vars.push_back("SCRIPT_NAME=" + request.getPath());
    {
        std::string uri = request.getPath();
        if (!request.getQueryString().empty()) uri += "?" + request.getQueryString();
        env_vars.push_back("REQUEST_URI=" + uri);
    }
    env_vars.push_back("DOCUMENT_ROOT=" + config.root_directory);

    // Opcional: si manejas POST hacia CGI
    if (!request.getHeaderValue("Content-Type").empty())
        env_vars.push_back("CONTENT_TYPE=" + request.getHeaderValue("Content-Type"));
    if (!request.getHeaderValue("Content-Length").empty())
        env_vars.push_back("CONTENT_LENGTH=" + request.getHeaderValue("Content-Length"));


    std::vector<char *> envp;
    for (unsigned long i = 0; i < env_vars.size(); i++)
        envp.push_back(strdup(env_vars[i].c_str()));
    envp.push_back(NULL);
    return (envp);
}

int manageCGI(Client &client, Response &response)
{
    (void)response;
    const Request &request = client.getRequest();
    const ServerConfig &config = client.getConfig();
    std::string full_path = config.root_directory + request.getPath();
    std::string extension = full_path.substr(full_path.rfind('.'));
    std::string execPath = MethodHandler::getMimeType(full_path);

    std::cout << YELLOW << "[INFO] Cgi management execPath: " << execPath << RESET << std::endl;

    int pipe_in[2];
    int pipe_out[2];
    if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0)
        return -1;

    pid_t pid = fork();
    if (pid == 0)
    {
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);

        close(pipe_in[1]);
        close(pipe_out[0]);

        char *argv[] = {(char *)execPath.c_str(), (char *)full_path.c_str(), NULL};
        std::vector<char *> env = setEnv(client);
        execve(argv[0], argv, env.data());
        exit(1);
    }
    else if (pid > 0)
    {
        // Proceso padre: no bloqueante + epoll
        close(pipe_in[0]);
        close(pipe_out[1]);

        // No bloqueante en stdout del CGI
        fcntl(pipe_out[0], F_SETFL, O_NONBLOCK);

        // Registrar el pipe de salida del CGI en epoll
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = pipe_out[0];
        epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, pipe_out[0], &ev);

        // Para GET no enviamos cuerpo al CGI; cerramos stdin del CGI
        close(pipe_in[1]);

        client.setCGIContext(pid, -1, pipe_out[0]);
        client.setState(CGI_RUNNING);
        return 0;
    }
    return -1;
}
