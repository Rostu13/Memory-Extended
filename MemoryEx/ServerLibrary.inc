#if defined _MemoryEx_ServerLibrary_include_
  #endinput
#endif
#define _MemoryEx_ServerLibrary_include_


#include <sdktools>

#include <MemoryEx/Stocks>

#include <MemoryEx/WindowsFunction>
#include <MemoryEx/LinuxFunction>

enum OS
{
	OS_Unknown, 

	OS_Windows,
	OS_Linux,
	OS_Mac
}

stock OS GetServerOS()
{
	static OS os; // default OS_Unknown;

	if(os != OS_Unknown)
	{
		return os;
	}
	//Prolog linux = ./srcds_linux -game csgo
	// windows = srcds.exe -game csgo

	char sProlog[4];
	GetCommandLine(sProlog, sizeof sProlog);

	if(sProlog[0] == '.')
	{
		os = OS_Linux;
	}
	else
	{
		os = OS_Windows;
	}

	return os;
}