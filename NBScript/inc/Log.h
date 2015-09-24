#pragma once
#include <string>
#include <fstream>

namespace NBE
{
	class Log
	{
	private:
		Log();
		~Log();
	public:
		static void Release();
		static void writeLog(std::string& str);
		static void writeLog(const char* str);
		static void writeLog(const char c);
	private:
		static std::ofstream* ofs;
	};
}