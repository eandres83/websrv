#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <iostream>
#include <sstream>
#include <ctime>
#include <string>

enum LogLevel
{
	INFO,
	TRACE,
	FATAL
};

class Logger
{
	private:
		Logger();
		static std::string _getTimestamp();

	public:
		static void log(LogLevel level, const std::string& message);
};

#endif // LOGGER_HPP
