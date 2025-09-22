/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nquecedo <nquecedo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/10 18:14:25 by nquecedo          #+#    #+#             */
/*   Updated: 2025/09/22 17:48:20 by nquecedo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CGI.hpp"

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
    env_vars.push_back("PATH=/home/nquecedo/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin:/home/nquecedo/.vscode/extensions/ms-python.debugpy-2025.10.0-linux-x64/bundled/scripts/noConfigScripts:/home/nquecedo/.config/Code/User/globalStorage/github.copilot-chat/debugCommand");

    // Rutas
    env_vars.push_back("SCRIPT_FILENAME=" + (config.root_directory + request.getPath()));
    env_vars.push_back("SCRIPT_NAME=" + request.getPath());
    {
        std::string uri = request.getPath();
        if (!request.getQueryString().empty()) uri += "?" + request.getQueryString();
        env_vars.push_back("REQUEST_URI=" + uri);
    }
    env_vars.push_back("DOCUMENT_ROOT=" + config.root_directory);

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
    std::string execPath = MethodHandler::getMimeType(full_path); // esto mover a cuadno necesite la direccion de cada lenguaje

    std::cout << YELLOW << "[INFO] Cgi management execPath: " << execPath << RESET << std::endl;
    // std::cout << GREEN << "" << request.getQueryString() << RESET << std::endl;

    //  1. Crear los pipes
    int pipe_in[2];
    int pipe_out[2];
    pipe(pipe_in);
    pipe(pipe_out);
    char buffer[4090];
    std::string cgi_output;
    int bytes_read;

    // 2. Crear el proceso hijo
    pid_t pid = fork();
    if (pid == 0)
    {
        // 3. Redireccionar stdin y stdout
        dup2(pipe_in[0], STDIN_FILENO);   // El stdin del hijo es la lectura del pipe_in
        dup2(pipe_out[1], STDOUT_FILENO); // El stdout del hijo es la escritura del pipe_out

        // 4. Cerrar los descriptores de fichero que no se usan en el hijo
        close(pipe_in[1]);
        close(pipe_out[0]);

        char *argv[] = {(char *)execPath.c_str(), (char *)full_path.c_str(), NULL};
        std::vector<char *> env = setEnv(client);

        execve(argv[0], argv, env.data());

        exit(1);
    }
    else if (pid > 0)
    {
        close(pipe_in[0]);
        close(pipe_out[1]);

        // AÑADIR LOGICA DE POST     TODO
        waitpid(pid, NULL, 0);

        while ((bytes_read = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
            cgi_output.append(buffer, bytes_read);

        if (bytes_read < 0)
        {
            perror("read");
        }

        close(pipe_in[1]);
        close(pipe_out[0]);
    }

    std::cout << GREEN << "AAAAAAA" << cgi_output << RESET <<std::endl;

    response.buildCustomResponse("200", "OK", cgi_output);
    response.addHeader("Content-Type", "text/html");
    return (0);
}
