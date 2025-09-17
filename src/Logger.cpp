#include "../includes/Logger.hpp"

std::string Logger::_getTimestamp()
{
	char		buffer[100];
	std::time_t	now = std::time(NULL);
	std::tm*	ltm = std::localtime(&now);

	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
	return (std::string(buffer));
}

void Logger::log(LogLevel level, const std::string& message)
{
	std::string levelStr;
	switch (level)
	{
		case INFO:
			levelStr = "[INFO] ";
			break;
		case TRACE:
			levelStr = "[TRACE] ";
			break;
		case FATAL:
			levelStr = "[FATAL] ";
			break;
	}

	std::cout << levelStr << "\t" << _getTimestamp() << " : " << message << std::endl;
}

