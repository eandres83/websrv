/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser_Server.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igchurru <igchurru@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/06 10:11:42 by igchurru          #+#    #+#             */
/*   Updated: 2025/10/06 16:41:36 by igchurru         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../includes/Config.hpp"

bool	Config::ParseErrorPageDirective(const std::string& content, size_t& index, ServerConfig& server)
{
	std::string	code_token;
	std::string	path_token;
	std::string	semicolon;
	int			error_code;
	
	code_token = GetNextToken(content, index);								//	Expect error code.
	if (code_token.empty())
	{
		std::cerr << "Error: Unexpected EOF after 'error_page' directive." << std::endl;
		return false;
	}
	std::istringstream code_ss(code_token);									// Convert code string to int
	code_ss >> error_code;
	if (code_ss.fail() || error_code < 300 || error_code > 599)				// Validation: Check conversion and range.
	{    
		std::cerr << "Error: Invalid or out-of-range error code '" << code_token << "'." << std::endl;
		return false;
	}
	path_token = GetNextToken(content, index);								//	Expect path.
	if (path_token.empty())													//	Validation: EOF
	{
		std::cerr << "Error: Unexpected EOF after error code (missing path)." << std::endl;
		return false;
	}
	if (path_token == ";")													//	Validation: Path is not ';'.
	{
		std::cerr << "Error: 'error_page' path is missing." << std::endl;
		return false;
	}
	if (server.error_pages.find(error_code) != server.error_pages.end())	//	Validation: Method .find() from map container to check double entries
	{
		std::cerr << "Error: Duplicate 'error_page' entry for code " << error_code << "." << std::endl;
		return false;
	}
	semicolon = GetNextToken(content, index);								//	Expect ';'
	if (semicolon != ";")
	{
		std::cerr << "Error: Expected ';' after error path, found '" << semicolon << "'" << std::endl;
		return false;
	}
	server.error_pages[error_code] = path_token;							//	All OK. Populate map.
	return true;
}


/*	Parses the 'reuse_addr on|off;' directive and sets the boolean flag. By default it is 'off'. */
bool Config::ParseReuseAddrDirective(const std::string& content, size_t& index, ServerConfig& server)
{
	std::string token;

	token = GetNextToken(content, index);
	if (token.empty())																//	Expect value.
	{
		std::cerr << "Error: Unexpected EOF after 'reuse_addr' directive." << std::endl;
		return false;
	}
	if (token == "on")																// Validate and assign.
		server.enable_reuse_addr = true;
	else if(token == "off")
		server.enable_reuse_addr = false;
	else
	{
		std::cerr << "Error: Expected 'on' or 'off' after 'reuse_addr'. Found '" << token << "'" << std::endl;
		return false;
	}
	token = GetNextToken(content, index);
	if (token != ";")																//	Expect ';'
	{
		std::cerr << "Error: Expected ';' after 'reuse_addr' value, found '" << token << "'" << std::endl;
		return false;
	}
	return true;    
}

/*	Parses 'client_max_body_size N[unit];' and converts it to bytes. */
bool Config::ParseClientMaxBodySizeDirective(const std::string& content, size_t& index, ServerConfig& server)
{
	std::string token;
	long long value = 0;
	char unit = 0;

	if (server.client_max_body_size != 1048576)						//	Validation: Check duplicates against the constructor default.
	{
		std::cerr << "Error: Duplicate 'client_max_body_size' directive found." << std::endl;
		return false;
	}
	token = GetNextToken(content, index);							//	Expect value
	if (token.empty())
	{
		std::cerr << "Error: Unexpected EOF after 'client_max_body_size' directive." << std::endl;
		return false;
	}
	std::string num_part = token;									//	Extract value and unit.
	if (token.length() > 0 && std::isalpha(token[token.length() - 1]))
	{
		unit = std::tolower(token[token.length() - 1]);				//	Separate unit
		num_part = token.substr(0, token.length() - 1);
	}
	std::istringstream ss(num_part);   								//	Convert numeric part using stringstream
	ss >> value;
	if (ss.fail() || value < 0)										//	Validation: Check conversion or negative
	{
		std::cerr << "Error: Invalid size value '" << token << "' in client_max_body_size." << std::endl;
		return false;
	}
	if (unit == 'k')												// Multiply based on unit.
		value *= 1024;
	else if (unit == 'm')
		value *= 1024 * 1024;
	else if (unit == 'g')
		value *= 1024 * 1024 * 1024;
	else if (unit != 0)												//	Invalid unit found
	{
		std::cerr << "Error: Invalid unit '" << unit << "' in client_max_body_size. Expected k, m, or g." << std::endl;
		return false;
	}
	server.client_max_body_size = value;							//	Populate
	token = GetNextToken(content, index);							//	Expect ';'
	if (token != ";")
	{
		std::cerr << "Error: Expected ';' after size value, found '" << token << "'" << std::endl;
		return false;
	}
	return true;
}

/*	Parses the 'listen <port>' directive and handles errors. */
bool Config::ParseListenDirective(const std::string& content, size_t& index, ServerConfig& server)
{
    std::string	token;
	int			temp_port;

    token = GetNextToken(content, index);								//	Expect value
	if (token.empty())													//	Validation: Check if empty.
    {
        std::cerr << "Error: Unexpected EOF after 'listen' directive." << std::endl;
        return false;
    }
    if (server.port != 0) 												//	Validation: Check if port is already set.
	{
        std::cerr << "Error: Duplicate 'listen' directive." << std::endl;
        return false;
    }
    std::istringstream ss(token);										//	String to int using stringstream
    ss >> temp_port;
    if (ss.fail() || temp_port <= 0 || temp_port > 65535)				//	Validation: Conversion and range (0-65535)
	{
        std::cerr << "Error: Invalid port number '" << token << "' in listen directive." << std::endl;
        return false;
    }
    server.port = temp_port;											//	Populate struct
    token = GetNextToken(content, index);								//	Expect ';'
    if (token != ";")
    {
        std::cerr << "Error: Expected ';' after 'listen' value, found '" << token << "'" << std::endl;
        return false;
    }
	return true;
}