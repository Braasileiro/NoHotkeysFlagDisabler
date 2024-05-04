#include "pch.h"

enum LogLevel {
	DEBUG,
	INFO,
	WARNING,
	EXCEPTION,
	CRITICAL
};

class Logger
{
private:
	std::ofstream log;

	std::string LevelName(LogLevel level)
	{
		switch (level) {
		case DEBUG:
			return "DEBUG";
		case INFO:
			return "INFO";
		case WARNING:
			return "WARNING";
		case EXCEPTION:
			return "EXCEPTION";
		case CRITICAL:
			return "CRITICAL";
		default:
			return "UNKNOWN";
		}
	}

	std::string CurrentTime()
	{
		char timestamp[20];

		tm info;
		time_t now = time(0);
		localtime_s(&info, &now);
		strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &info);

		return std::string(timestamp);
	}
public:
	Logger()
	{
		log.open("NoHotkeysFlagDisabler.log", std::ios::trunc);

		if (!log.is_open()) {
			std::cerr << "Failed to open log file." << std::endl;
		}
	}

	~Logger() {
		log.close();
	}

	void Log(LogLevel level, const std::string& message)
	{
		// Log entry
		std::ostringstream entry;

		entry
			<< "[" << CurrentTime() << "] "
			<< LevelName(level)
			<< ": "
			<< message
			<< std::endl;

		// Console output
		std::cout << entry.str();

		// File output
		if (log.is_open()) {
			log << entry.str();

			// Immediate write to file 
			log.flush();
		}
	}
};

// Global Logger
extern Logger logger;
