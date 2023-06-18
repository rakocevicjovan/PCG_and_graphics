#pragma once
#include "FileUtilities.h"

// A thought, this could be a static function API with a #define or const path, 
// which would avoid singleton shenanigans and would works basically the same for simple uses

// A singleton, one and only in my code. One could even say it's a single singleton...
class Logger
{
private:

	std::string _path;

	Logger() = default;
	Logger(Logger&&) = default;
	Logger& operator=(Logger&& other) = default;
	~Logger() = default;

	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

public:

	// Lazy load, instantiated on first use.
	static Logger& getInstance()
	{
		// Not thread safe, unless it's "primed" by a guaranteed single thread call
		static Logger instance;		
		return instance;			
	}

	static void Log(const std::string& line)
	{
		// Pretty damn sure this needs a lock
		FileUtils::writeAllBytes("Logger.txt", line.data(), line.size() * sizeof(char), std::ios::app);
	}
};
