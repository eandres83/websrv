/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nquecedo <nquecedo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/10 18:14:25 by nquecedo          #+#    #+#             */
/*   Updated: 2025/09/17 16:05:30 by nquecedo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CGI.hpp"

int manageCGI(Client &client, Response &response)
{
    (void)response;
    const Request& request = client.getRequest();
	const ServerConfig& config = client.getConfig();
	std::string full_path = config.root_directory + request.getPath();
    std::string execPath = MethodHandler::getMimeType(full_path);

    std::cout << YELLOW << "[INFO] Cgi management execPath: " << execPath << RESET <<std::endl;

    return (0);
}
