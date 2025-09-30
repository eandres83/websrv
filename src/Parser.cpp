/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igchurru <igchurru@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/30 13:14:04 by igchurru          #+#    #+#             */
/*   Updated: 2025/09/30 15:50:58 by igchurru         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../includes/Config.hpp"

/*	Extracts the next valid token from the raw config data (C++98 Safe).
 *	raw_data = The entire configuration file content formatted as a std::string
 *	current = A reference to the current position in raw_data.
 *	returns: Next token as a std::string. Empty string when the end is reached. */
std::string GetNextToken(const std::string& raw_data, size_t& current)
{
    const size_t		datalen = raw_data.length();
	const std::string	delimiters = " \t\n\r{};#";
	size_t				start;
	size_t				end;
	char				next_char;
	
    while (current < datalen)											//	1.- SKIPPING LOOP: Skip all blanks, tabs, newlines, carriage returns.
    {												
        while (current < datalen && (raw_data[current] == ' ' || raw_data[current] == '\t' ||  raw_data[current] == '\n' || raw_data[current] == '\r'))	
            current++;
        if (current < datalen && raw_data[current] == '#')				//	Skip '#' (comments) to end of line
        {
            while (current < datalen && raw_data[current] != '\n')		//	When newline, restart outer loop.
                current++;
            continue; 
        }
        if (current < datalen)											//	If non-separator, break skipping loop.
            break;
    }
    if (current >= datalen)												//	Check if EOF after skipping separators
        return "";
    next_char = raw_data[current];										//	2.- EXTRACT SINGLE CHAR: Delimiter tokens
    if (next_char == '{' || next_char == '}' || next_char == ';')
    {
        current++; 														//	Advance, 
		return std::string(1, next_char);								//	then create and return a single-char string (C++98 style).
	}
	start = current;													//	3. EXTRACT MULTI-CHAR TOKENS: Directives or Values
	end = raw_data.find_first_of(delimiters, current);					//	Find position of first delimiter starting from 'current'. If none found, npos.
    if (end != std::string::npos)										//	If delimiter, token ends there.
        current = end; 													//	Update 'current' to delimiter
    else																//	If no delimiter (npos), rest of the string is the token.
        current = datalen;
    return raw_data.substr(start, current - start);						// Extract and return the token using substr (len = current - start)
}

/*	This function uses standard C++ file streams to read the file contents.
 	Return: whole file as a std::string. Empty string on failure. */
std::string ReadFileToString(const std::string& filename)
{
	std::stringstream	buffer;
	std::string			contents;
	
	std::ifstream file(filename.c_str(), std::ios::in);					//	.c_str() extracts data from filename as a stream. Guarantees null termination.
	if (!file.is_open())
	{
		std::cerr << "Error: Cannot open configuration file" << std::endl;
		return "";
	}
	buffer << file.rdbuf();												//	Store the whole stream in a buffer...
	file.close();														//	close the file to avoid leaking...
	contents = buffer.str();											//	then convert the buffer to a string.
	return contents;
}

bool	parse(const char* filename)
{
	std::string	contents;
	std::string token;
	
	contents = ReadFileToString(filename);
	//	GetNextToken
}
