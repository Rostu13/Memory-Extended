#if defined _MemoryEx_ServerLibrary_include_
  #endinput
#endif
#define _MemoryEx_ServerLibrary_include_


#include <sdktools>

#include <MemoryEx/Stocks>

#include <MemoryEx/WindowsFunction>
#include <MemoryEx/LinuxFunction>

enum struct ModuleInfo
{
	Address base;
	int size;
}

static const char g_sExts[][] = 
{
	"",

	".dll",
	".so",
	".so"
}

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
stock StringMap GetListLibraries()
{
	return GetServerOS() == OS_Windows ? WindowsGetDllList() : LinuxParseMapsFile();
}
stock Address GetModuleHandle(const char[] sName)
{
	ModuleInfo info;

	if(!GetModuleInfo(sName, info))
	{
		return Address_Null;
	}

	return info.base;
}
stock any GetModuleSize(const char[] sName)
{
	ModuleInfo info;

	if(!GetModuleInfo(sName, info))
	{
		return 0;
	}

	return info.size;
}
stock Address GetModuleEnd(const char[] sName)
{
	ModuleInfo info;

	if(!GetModuleInfo(sName, info))
	{
		return Address_Null;
	}

	return info.base + view_as<Address>(info.size);
}
stock bool GetModuleInfo(const char[] sNameLibrary, ModuleInfo info)
{
	OS os = GetServerOS();

	int iLength = strlen(sNameLibrary);
	int iLengthExts = strlen(g_sExts[os]);
	int iLengthName = iLength + iLengthExts + 1;

	char[] sName = new char[iLengthName];

	strcopy(sName, iLengthName, sNameLibrary);

	if(iLength > 0x04) // mem.lib.InitModule("engine"); "engine" convert to "engine.so" or "engine.dll"
	{
		if(strcmp(sName[iLength - iLengthExts], g_sExts[os]) != 0)
		{
			FormatEx(sName, iLengthName, "%s%s", sNameLibrary, g_sExts[os]);
		}
	}

	if(!GetListLibraries().GetArray(sName, info, sizeof(ModuleInfo)))
	{
		return false;
	}

	return true;
}
stock Address GetProcAddress(const char[] sLibrary, const char[] sName )
{
	Address pBase = GetModuleHandle(sLibrary);

	if(pBase == Address_Null)
	{
		return Address_Null;
	}

	return GetServerOS() == OS_Windows ? GetProcAddressWindows(pBase, sName) : GetProcAddressLinux(pBase, sName);
}