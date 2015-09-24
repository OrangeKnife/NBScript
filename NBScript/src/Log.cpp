#include "Log.h"
namespace NBE
{
	Log::Log()
	{}

	Log::~Log()
	{}

	void Log::Release()
	{
		if (ofs)
		{
			ofs->close();
			delete ofs;
		}
	}

	void Log::writeLog(std::string& str)
	{
		if (!ofs)
			ofs = new std::ofstream("Log.txt");

		ofs->write(str.c_str(), str.size());	
	}

	void Log::writeLog(const char* str)
	{
		writeLog(std::string(str));
	}

	void Log::writeLog(const char c)
	{
		if (!ofs)
			ofs = new std::ofstream("Log.txt");

		ofs->write(&c, 1);

	}

	std::ofstream* Log::ofs;
}
