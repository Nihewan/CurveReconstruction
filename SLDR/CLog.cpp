#include "StdAfx.h"
#include "CLog.h"
#include <iostream>
#include <stdarg.h>
#include <fstream>

CLog::CLog(void)
{
}

CLog::~CLog(void)
{
}

void CLog::initConsole() {
// #ifdef _DEBUG
	AllocConsole();
	freopen("CON", "r", stdin );
	freopen("CON", "w", stdout);
	freopen("CON", "w", stderr);
	std::cout << "Log information(Only showed in debug mode):\n";
// #endif // _DEBUG
}

void CLog::freeConsole() {
// #ifdef _DEBUG
	FreeConsole();
// #endif // _DEBUG
}

/*
Output Formatting string
%f for double
%d for int
*/
void CLog::log(const char* format, ...) {
	va_list argPtr;
	va_start(argPtr, format);
	char ch;

	while(ch=*format++)   
	{   
		if (ch == '%') {
			ch = *format++;
			switch(ch)   
			{   
			case 'f':   
				{   
					double dNum = va_arg(argPtr, double);   
					std::cout << dNum;   
					break;   
				}   
			case 'd':   
				{   
					int iNum = va_arg(argPtr, int);   
					std::cout << iNum;  
					break;   
				}   
			default:   
				putchar(ch);   
			}   
		}
		else {
			putchar(ch); 
		}
	}   
	std::cout << std::endl;
}

void CLog::logFile(const char* fileName, const char* content){
	using namespace std;
	ofstream file;
	file.open(fileName);
	file<<content;
	file.close();
}

