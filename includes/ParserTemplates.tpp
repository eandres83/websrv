//	TEMPLATE FUNCTIONS FOR PARSING DIRECTIVES SHARED BOTH BY SERVER AND LOCATION

#include <string>

template <typename ConfigT>
bool	ParseAutoindexDirectiveT(const std::string& content, size_t& index, ConfigT& config_struct)
{
	std::string	token;

	token = GetNextToken(content, index);
	if (token.empty())
	{
		std::cerr << "Error: Unexpected EOF after 'autoindex' directive" << std::endl;
		return false;
	}
	if (token == "on")
		config_struct.autoindex = true;
	else if(token == "off")
		config_struct.autoindex = false;
	else
	{
		std::cerr << "Error: Expected 'on' or 'off' after 'autoindex' directive. Found '" << token << "'" << std::endl;
		return false;
	}
	token = GetNextToken(content, index);
	if (token != ";")
	{
		std::cerr << "Error: Expected ';' after 'autoindex' value, found '" << token << "'" << std::endl;
		return false;
	}
	return true;	
}

template <typename ConfigT>
bool	ParseRootDirectiveT(const std::string& content, size_t& index, ConfigT& config_struct)
{
	std::string	token;

	token = GetNextToken(content, index);
	if (token.empty())													//	Validation: Check if EOF.
	{
		std::cerr << "Error: Unexpected EOF after 'root' directive." << std::endl;
		return false;
	}
	if (!config_struct.root_directory.empty())									//	Validation: Check if root already set.
	{
		std::cerr << "Error: Duplicate 'root' directive." << std::endl;
		return false;
	}
	config_struct.root_directory = token;										//	Populate struct. No need for conversion since 'token' is already a string.
	token = GetNextToken(content, index);								//	Expect ';'
	if (token != ";")
	{
		std::cerr << "Error: Expected ';' after 'root' value, found '" << token << "'" << std::endl;
		return false;
	}
	return true;
}

template <typename ConfigT>
bool	ParseAllowedMethodsDirectiveT(const std::string& content, size_t& index, ConfigT& config_struct)
{
	std::string token;

	if (!config_struct.allowed_methods.empty())											//	Validation. "One-and-done"
	{
		std::cerr << "Error: Duplicate 'allowed_methods' directive found." << std::endl;
		return false;
	}
	while (!(token = GetNextToken(content, index)).empty())							//	Loop untill ';'
	{
		if (token == ";")															//	If end of directive...
		{
			if (config_struct.allowed_methods.empty())										//	... at least one valid method must be present
			{
				std::cerr << "Error: 'allowed_methods' directive requires at least one verb." << std::endl;
				return false;
			}
			return true;
		}
		if (!(token == "GET" || token == "POST" || token == "DELETE"))				//	Validation: Only uppercase.
		{
			std::cerr << "Error: Invalid HTTP method '" << token << "' in allowed_methods. Must be GET, POST, or DELETE." << std::endl;
			return false;
		}
		config_struct.allowed_methods.push_back(token);									//	Store valid method into the vector.
	}
	std::cerr << "Error: Unexpected EOF while parsing 'allowed_methods' directive (missing ';'?)." << std::endl;
	return false;
}