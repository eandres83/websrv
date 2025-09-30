/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igchurru <igchurru@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 13:14:04 by igchurru          #+#    #+#             */
/*   Updated: 2025/09/30 13:52:26 by igchurru         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../includes/Config.hpp"

std::string GetNextToken(const std::string& raw_data, size_t& current)
{
	
}

/*	This function uses standard C++ file streams to read the file contents.
 	Return: whole file as a std::string. Empty string on failure. */
std::string ReadFileToString(const std::string& filename)
{
	std::stringstream	buffer;
	std::string			contents;
	
	std::ifstream file(filename.c_str(), std::ios::in);		//	.c_str() extracts data from filename as a stream. Guarantees null termination.
	if (!file.is_open())
	{
		std::cerr << "Error: Cannot open configuration file" << std::endl;
		return "";
	}
	buffer << file.rdbuf();									//	Store the whole stream in a buffer...
	file.close();
	contents = buffer.str();								//	then convert it to a string.
	return contents;
}

bool	parse(const char* filename)
{
	//	ReadFileToString
	//	GetNextToken
}
