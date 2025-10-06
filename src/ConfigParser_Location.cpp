/******************************************************************************/
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser_Location.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: igchurru <igchurru@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/06 10:12:35 by igchurru          #+#    #+#             */
/*   Updated: 2025/10/06 11:01:19 by igchurru         ###   ########.fr       */
/*                                                                            */
/******************************************************************************/

#include "../includes/Config.hpp"

//	FUNCTIONS FOR PARSING LOCATION-SPECIFIC DIRECTIVES


//	CONSULTAR: UN ÚNICO RETURN CODE POR LOCATION? SI PUEDE HABER VARIOS SERÍA CLARAMENTE MEJOR UN MAP. Habría que cambiar la Config class.
bool Config::ParseReturnCodeDirective(const std::string& content, size_t& index, LocationConfig& current_location)
{
	std::string	token;
	int			code;

	token = GetNextToken(content, index);			//	Expect code
	std::stringstream ss(token);					//	Get into stream
	ss >> code;										//	Insert into int
}	

/*	Parses the 'index file1 file2 ... ;' directive. */
bool Config::ParseIndexDirective(const std::string& content, size_t& index, LocationConfig& current_location)
{
	std::string token;

	if (!current_location.index_files.empty())		//	Validation: One-and-done
	{
		std::cerr << "Error: Duplicate 'index' directive found in location block." << std::endl;
		return false;
	}
	while (!(token = GetNextToken(content, index)).empty())		//	Loop untill ';'
	{
		if (token == ";")
		{
			if (current_location.index_files.empty())			//	At end, must not be empty.
			{
				std::cerr << "Error: 'index' directive requires at least one file name." << std::endl;
				return false;
			}
			return true;										//	If end, and valid token previously stored, it is OK.
		}
		if (token == "{" || token == "}")
		{
			std::cerr << "Error: Unexpected block delimiter '" << token << "' inside 'index' directive." << std::endl;
			return false;
		}
		current_location.index_files.push_back(token);
	}
	std::cerr << "Error: Unexpected EOF while parsing 'index' directive (missing ';'?)." << std::endl;   	//	If loop exited because GetNextToken returned empty, it means EOF.
	return false;
}
