#pragma once
class CLog
{
public:
	CLog(void);
	~CLog(void);
public:
	static void log(const char* format, ...);
	static void logFile(const char* fileName, const char* content);
	static void initConsole();
	static void freeConsole();
};
