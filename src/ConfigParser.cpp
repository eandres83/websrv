/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igchurru <igchurru@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 13:14:04 by igchurru          #+#    #+#             */
/*   Updated: 2025/10/03 14:04:31 by igchurru         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../includes/Config.hpp"

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
	if (server.enable_reuse_addr != false) 											//	Validation: No duplicates. This is tricky for booleans.
	{
		std::cerr << "Error: Duplicate 'reuse_addr' directive found." << std::endl;
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

/*	Parses the 'listen' directive and handles errors.
 *	server: The ServerConfig struct being populated.  */
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

// Private member function of Config class

/*	Parses a single 'location path { ... }' block.
	Expects to be called immediately AFTER the "location" token has been read
	return true if the block was successfully parsed, false on syntax error. */
bool Config::ParseLocationBlock(const std::string& content, size_t& index, ServerConfig& server)
{
	LocationConfig 	current_location;
	std::string		token;

	token = GetNextToken(content, index);						// Expect path.
	if (token.empty() || token == "{")
	{
		std::cerr << "Error: Expected path after 'location' directive." << std::endl;
		return false;
	}
	// TODO: Add validation for path format
	current_location.path = token;								// Store path
	token = GetNextToken(content, index);						//	Expect '{'
	if (token != "{")
	{
		std::cerr << "Error: Expected '{' after location path, found '" << token << "'" << std::endl;
		return false;
	}
	while (!(token = GetNextToken(content, index)).empty())		//	Parse directives untill '}' is found.
	{
		if (token == "}")										//	If '}', break the loop.
			break; 
		else if (token == "root")
		{
			if (!ParseRootDirectiveT(content, index, current_location)) 
				return false;
		}
		else if (token == "autoindex")
		{
			if (!ParseAutoindexDirectiveT(content, index, current_location)) 
				return false;
		}
		else if (token == "allowed_methods")
		{
			if (!ParseAllowedMethodsDirectiveT(content, index, current_location)) 
			return false;
		}
		// TODO: Add location-specific directives like 'index', 'return', etc.)
		else
		{
			std::cerr << "Error: Unknown directive '" << token << "' inside location block." << std::endl;
			return false;
		}
	}
	if (token.empty())
	{
		std::cerr << "Error: Unexpected end of file while parsing location block (missing '}')" << std::endl;
		return false;
	}
	server.locations.push_back(current_location);				// If all is OK, attach location block to parent server
	return true;
}

bool	Config::ParseServerBlock(const std::string& content, size_t& index)
{
	ServerConfig	new_server;								//	Instantiate a new ServerConfig struct to be filled and added to class Config.
	std::string		token;

	token = GetNextToken(content, index);					//	Expect opening brace.
	if (token != "{")
	{
		std::cerr << "Error: expected '{' token after 'server'. Found " << token << std::endl;
		return false;
	}
	while (!(token = GetNextToken(content, index)).empty())	//	Loop untill closing brace.
	{
		if (token == "}")
		{
			break;
		}
		else if (token == "location")
		{
			// NESTED BLOCK: Call a dedicated function to handle 'location { ... }'
            // The location config will be pushed back into current_server.locations inside this helper.
		}
		else if (token == "listen")
		{
			if (!ParseListenDirective(content, index, new_server))
				return false;
		}
		else if (token == "root")
		{
			if (!ParseRootDirectiveT(content, index, new_server))
				return false;
		}
		else if (token == "error_page")
		{
			if (!ParseErrorPageDirectiveT(content, index, new_server))
				return false;
		}
		else if (token == "autoindex")
		{
			if (!ParseAutoindexDirectiveT(content, index, new_server))
				return false;
		}
		else if (token == "upload_path")
		{
			if (!ParseReuseAddrDirective(content, index, new_server))
				return false;
		}
		else
		{
			std::cerr << "Error: Unknown directive " << token << " found inside server block" << std::endl;
			return false;
		}
	}
	_server_configs.push_back(new_server);		//	If all is OK, add new_server to configurations vector in main Config class.
	return true;
}

bool	Config::parse(const char* filename)
{
	std::string	content;
	std::string token;
	size_t		index = 0;

	content = ReadFileToString(filename);
	while (!(token = GetNextToken(content, index)).empty())
	{
		if (token == "server")
		{
			if (!ParseServerBlock(content, index))
				return false;
		}
		else 
		{
			std::cerr << "Error: Unexpected token '" << token << "' at top level." << std::endl;
			return false;
		}
	}
	
		// ... (Final validation, e.g., checking for unique ports)
	
	return true;
}
