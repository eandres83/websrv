#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <string>

class Config
{
	private:
		int		_port;
		std::string	_root_directory;
		std::string	_error_page_404;
		bool		_autoindex;
		int		_enable_reuse_addr;

	public:
		Config();
		~Config();

		bool			parse(const char* filename);
		int			getPort() const;
		const std::string&	getRootDirectory() const;
		const std::string&	getErrorPage404() const;
		bool			getAutoindex() const;
		int				getReuseAddr() const;
};

#endif // CONFIG_HPP
