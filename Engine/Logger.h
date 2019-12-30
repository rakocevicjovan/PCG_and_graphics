#pragma once

//@TODO... singleton sorted out, will make functionality later
class Logger
{
private:
	Logger() {}

public:
	static Logger& getInstance()
	{
		static Logger instance;		// Guaranteed to be destroyed.
		return instance;			// Instantiated on first use.
	}


	//prevents copying and assignment, as this is a singleton, one and only in my code
	//one could say it's a single singleton...
	Logger(Logger const&) = delete;
	void operator=(Logger const&) = delete;
};