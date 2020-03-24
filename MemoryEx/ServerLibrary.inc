#if defined _MemoryEx_ServerLibrary_include_
  #endinput
#endif
#define _MemoryEx_ServerLibrary_include_


#include <sdktools>

#include <MemoryEx/WindowsFunction>
#include <MemoryEx/LinuxFunction>

#include <MemoryEx/Stocks>

enum OSType
{
	OSType_Windows,
	OSType_Windows64,
	OSType_Linux,
	OSType_Linux64,
	OSType_Mac,
	OSType_Mac64
}
static const char g_sOSType[][] = 
{
	"windows",
	"windows64", 
	"linux",
	"linux64",
	"mac",
	"mac64"
};

enum OS
{
	OS_Unknown, 

	OS_Windows,
	OS_Linux,
	OS_Mac
}

static StringMap g_hGetOSResult;
static bool g_bRequestSV;

static Address 	g_ServerDLLBase;
static int 		g_ServerDLLSize;

static const int g_iPEBHeader[] = {0x4D, 0x5A, 0x90, 0x00};
static const int g_iELFHeader[] = {0x7F, 0x45, 0x4C, 0x46}; // bad :)

public SMCResult GetOS_NewSection(SMCParser smc, const char[] name, bool opt_quotes)
{
	if(g_bRequestSV)
	{
		return SMCParse_Halt;
	}

	if(!strcmp(name, "sv"))
	{
		g_bRequestSV = true;
	}

	return SMCParse_Continue;
}
public SMCResult GetOS_KeyValue (SMCParser smc, const char[] key, const char[] value, bool key_quotes, bool value_quotes)
{
	if(g_bRequestSV)
	{
		g_hGetOSResult.SetValue(key, StringToInt(value));
	}
	return SMCParse_Continue;
}
stock OS GetServerOS()
{
	static OS os; // default OS_Unknown;

	if(os != OS_Unknown)
	{
		return os;
	}
	int iOffset;

	/*
		Remind you of the philosophy of this plugin - don't need create any files. 
		This offset is ideal for us, because there are different values ​​for each of the OS
		"Offsets"
		{
			"sv"
			{
				"windows"	"8"
				"linux"		"13"
				"linux64"	"12"
				"mac64"		"9"
			}
		}
	*/

	GameData hGame = new GameData("sdktools.games/engine.csgo");
	iOffset = hGame.GetOffset("sv");
	delete hGame;

	if(iOffset == -1)
	{
		LogStackTrace("Couldn't get sv offset => sdktools.games/engine.csgo ");
		return os;
	}

	g_bRequestSV = false;

	if(g_hGetOSResult != null) delete g_hGetOSResult;
	g_hGetOSResult = new StringMap();

	char sPath[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, sPath, sizeof sPath, "gamedata/sdktools.games/engine.csgo.txt");

	if(!FileExists(sPath))
	{
		LogStackTrace("Couldn't open => %s", sPath);
		return os;
	}

	// KeyValues ​​don't adequately accept such files, therefore, so for get  offset was selected method via the SMCParser
	SMCParser parser = new SMCParser();
	parser.OnEnterSection = GetOS_NewSection;
	parser.OnKeyValue = GetOS_KeyValue;

	SMCError err = parser.ParseFile(sPath);

	if(err != SMCError_Okay)
	{
		char sError[256];
		SMC_GetErrorString(err, sError, sizeof sError);
		LogStackTrace("GetServerOs ParseFile failed - %s", sError);

		delete parser;
		return os;
	}

	int iValue;

	/*
		You'r OS is linux? - Need number = 13				g_hGetOSResult contains
		GetValue(g_sOSType[OSType_Windows] = 			"windows"	8		Is need number? - No			
		GetValue(g_sOSType[OSType_Linux] =				"linux"		13		Is need number? - Yes => Break;
														"linux64"	12
														"mac64"		9
	*/

	for(int x = 0; x < sizeof(g_sOSType); x++)
	{
		if(!g_hGetOSResult.GetValue(g_sOSType[x], iValue)) 
		{
			continue;
		}

		if(iValue == iOffset)
		{
			switch(view_as<OSType>(x))
			{
				case OSType_Windows, OSType_Windows64: 	os = OS_Windows;
				case OSType_Linux, OSType_Linux64: 		os = OS_Linux;
				case OSType_Mac, OSType_Mac64 : 		os = OS_Mac;
			}
			break;
		}
	}

	delete parser;
	return os;
}

static bool InitServerDLL()
{
	GameData hServer = new GameData("sdkhooks.games/engine.csgo");

	//The address of any function was received from the server.dll, with it we get to the beginning of the dll
	Pointer start = hServer.GetAddress("EntityListenersPtr");
	delete hServer;

	//PrintToServer("InitServerDLL entityptr = 0x%X", start);

	if(start == nullptr) return false;

	g_ServerDLLBase = nullptr;

	int signature[4];
	static int length = sizeof(signature);

	OS os = GetServerOS();

	if(os == OS_Windows)
	{
		signature = g_iPEBHeader;
	}
	else if(os == OS_Linux)
	{
		signature = g_iELFHeader;
	}
	else
	{
		ThrowError("InitServerDLL for OS_Unknown/OS_Mac - missing");
		return false;
	}

	char sBuffer[8];

	/* Reserse signature 
	0x4d 0x5a 0x90 0x00 => 0x00905A4D => 0x90954D = find value
	*/
	for(int x = length - 1; x >= 0; x--)
	{
		Format(sBuffer, sizeof sBuffer, "%s%X", sBuffer, signature[x]);
	}

	int x;
	int byte;
	int iValue;
	int iValueFind = HexToDec(sBuffer);

	Pointer adr;

	while(x != -1) //endless cycle
	{
		adr = start - PTR(x);
		byte = LoadFromAddress(adr, NumberType_Int8);

		if(byte == signature[length - 1]) // windows start dll = 0x4d 0x5a 0x90 0x00 => find 0x00
		{
			/* You are here ↓ 			 ↓ Change address and you are here
			0x4d 0x5a 0x90 0x00			0x4d 0x5a 0x90 0x00
			*/
			adr = adr - PTR(0x03); 
			iValue = LoadFromAddress(adr, NumberType_Int32);

			if(iValue == iValueFind)
			{
				g_ServerDLLBase =  adr;

				if(os == OS_Windows)
				{
					WindowsParsePEBHeader(adr, g_ServerDLLSize);
				}
				else // linux
				{
					//LinuxParseELFHeader(adr, g_ServerDLLSize);
				}

				break;
			}
		}

		x++;
	}

	if(g_ServerDLLBase == nullptr)
	{
		ThrowError("InitServerDLL => failed. LastValue [%d] == Need[%d]", iValue, iValueFind);
		return false;
	}

	return true;
}

stock Address GetServerDLLBase()
{
	if(g_ServerDLLBase == Address_Null)
	{
		InitServerDLL();
	}

	return g_ServerDLLBase;
}
stock int GetServerDLLSize()
{
	if(g_ServerDLLSize == 0)
	{
		InitServerDLL();
	}
	
	return g_ServerDLLSize;
}
