/*
	[Расширяем возможности SourcePawn по работе с памятью]
				Memory Extendвd

				Автор: Rostu [vk.com/rostu13]
				Версия: 1.0

	18.12.2019					Первый релиз [Windows Only]
*/

#include <sdktools>

#define Pointer Address
#define nullptr Address_Null

#define PTR(%0) view_as<Pointer>(%0)

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

enum struct ModuleInfo
{
	Pointer base;
	int size;
}

enum ByteType
{
	ByteType_Byte,
	ByteType_Word,
	ByteType_Int
}
enum struct ModifByte
{
	Pointer source;
	ByteType type;

	int original;
	int current;
}

static StringMap g_hGetOSResult;
static bool g_bRequestSV;

static OS g_ServerOS;
static ModuleInfo g_ServerDLL;

static const int g_iPEBHeader[] = {0x4D, 0x5A, 0x90, 0x00};
//static const int g_iELFHeader[] = {0x7F, 0x45, 0x4C, 0x46};


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
	OS os; // default OS_Unknown;
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
		LogError("Couldn't open => %s", sPath);
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
void InitServerOS()
{
	g_ServerOS = GetServerOS();
}

static bool InitServerDLL()
{
	GameData hServer = new GameData("sdkhooks.games/engine.csgo");

	//The address of any function was received from the server.dll, with it we get to the beginning of the dll
	Pointer start = hServer.GetAddress("EntityListenersPtr");
	delete hServer;

	if(start == nullptr) return false;

	g_ServerDLL.base = nullptr;

	int signature[4];
	static int length = sizeof(signature);

	if(g_ServerOS == OS_Windows)
	{
		signature = g_iPEBHeader;
	}
	//else if(g_ServerOS == OS_Linux)
	//{
	//	signature = g_iELFHeader;
	//}
	else
	{
		LogStackTrace("InitServerDLL for OS_Unknown/OS_Linux/OS_Mac - missing");
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
				g_ServerDLL.base =  adr;

				int iSize;

				if(g_ServerOS == OS_Windows)
				{
					int iOffset = LoadFromAddress(adr + PTR(0x3C), NumberType_Int32); // NT headers start
					iSize = LoadFromAddress(adr + PTR(iOffset + 0x50), NumberType_Int32); // NT->OptionalHeader.SizeOfImage
				}
				else // linux
				{

				}

				g_ServerDLL.size = iSize;

				break;
			}
		}

		x++;
	}

	if(g_ServerDLL.base == nullptr)
	{
		LogStackTrace("InitServerDLL => failed. LastValue [%d] == Need[%d]", iValue, iValueFind);
		return false;
	}

	return true;
}

enum
{
	MemoryEx_NoNeedAdd = (1 << 0),

	/*
		When will read from memory is started, MemoryEx will automatically add to this.pAddrBase. 

		...
		this.SetAddr(0x200) =>
		int byte  = this.ReadWord(0x20) [Read something]
		Now this.GetAddr return 0x222 => 0x200 [Base] + 0x20 [offset] + 0x02 [size type]
		...
	*/
	MemoryEx_AddAfterRead = (1 << 1),
	MemoryEx_AddAfterWrite = (1 << 2),
}

#define CHECKTOINIT(%0) if (!this.bInit){LogStackTrace("MemoryEx wasn't be initialized"); %0;}

enum struct MemoryEx
{
	Pointer pAddrBase;
	OS os;

	bool bSaveBytes;

	bool bInit;

	ModifByte lastModif;
	ArrayList hModifList;

	StringMap hModules;

	bool Init()
	{
		if(this.bInit) 
		{
			return true;
		}

		InitServerOS();

		if(g_ServerOS == OS_Unknown)
		{
			LogStackTrace("Coudn't get Server OS :(");
			return false;
		}

		if(!InitServerDLL())
		{
			return false;
		}

		if(this.hModifList != null) delete this.hModifList;
		if(this.hModules != null) delete this.hModules;

		this.hModifList = new ArrayList(sizeof(ModifByte));
		this.hModules = new StringMap();

		this.pAddrBase 	= nullptr;
		this.os 		= g_ServerOS;
		this.bSaveBytes = false;
		this.bInit 		= true;

		this.hModules.SetArray("server", g_ServerDLL, sizeof(ModuleInfo));

		return true;
	}
	void SetAddr(any address)
	{
		if(address < nullptr) 
		{
			return;
		}

		this.pAddrBase = PTR(address);
	}
	Pointer GetAddr()
	{
		return this.pAddrBase;
	}

	void Add(any iOffset)
	{
		this.pAddrBase += iOffset;
	}

	void SaveBytes(bool save)
	{
		this.bSaveBytes = save;
	}
	bool NeedSave()
	{
		return this.bSaveBytes;
	}
	void RestoreBytes()
	{
		ModifByte info;

		for(int x = 0; x < this.hModifList.Length; x++)
		{
			this.hModifList.GetArray(x, info, sizeof(ModifByte));
			StoreToAddress(info.source, info.original, info.type == ByteType_Byte ? NumberType_Int8 : info.type == ByteType_Word ? NumberType_Int16 : NumberType_Int32);
		}

		this.hModifList.Clear();
	}

	void ChangeSettings(bool bSave = true) // False = restore settings
	{
		static Pointer addr;
		static bool saveBytes;

		if(bSave)
		{
			addr = this.GetAddr();
			saveBytes = this.NeedSave();
		}
		else
		{
			this.SetAddr(addr);
			this.SaveBytes(saveBytes);
		}	
	}

	bool InitModule(const char[] sName)
	{
		ModuleInfo info;

		info.base = this.GetModuleHandle(sName);

		if(info.base == nullptr)
		{
			return false;
		}

		int iOffset = LoadFromAddress(info.base + PTR(0x3C), NumberType_Int32); // NT headers start
		info.size = LoadFromAddress(info.base + PTR(iOffset + 0x50), NumberType_Int32); // NT->OptionalHeader.SizeOfImage

		this.hModules.SetArray(sName, info, sizeof(ModuleInfo));

		return true;
	}
	
	Pointer GetModuleHandle(const char[] name)
	{
		/*
			push [string address] 	
			call dword ptr [GetModuleHandleW]
			retn							
		*/

		CHECKTOINIT(return nullptr)

		this.ChangeSettings();
		this.bSaveBytes = false;

		Pointer pKernelStr = this.FindUnicodeString("server", "kernel32.dll");
		Pointer module = this.FindValue("server", pKernelStr, 0xFF)  + PTR(0x06);

		if(pKernelStr == nullptr || module == nullptr) 
		{
			this.ChangeSettings(false);
			LogStackTrace("GetModuleHandles failed -> Base = 0x%X pKernelStr 0x%X module 0x%X end = 0x%X", g_ServerDLL.base, pKernelStr, module, g_ServerDLL.base + PTR(g_ServerDLL.size) );
			return nullptr;
		}

		module = PTR(LoadFromAddress(module, NumberType_Int32));

		//PrintToServer("GetModuleHandles suss -> Base = 0x%X pKernelStr 0x%X [offset 0x%X] module 0x%X [offset 0x%X] end = 0x%X", g_ServerDLL.base, pKernelStr, pKernelStr - g_ServerDLL.base, module, module - g_ServerDLL.base, g_ServerDLL.base + g_ServerDLL.size );

		static int offsetForString = 0x10; // offset between string and function
		static int offsetForEnd = 0x100; 

		int iLengthStr = strlen(name);

		this.SetAddr((view_as<int>(g_ServerDLL.base) + g_ServerDLL.size) - offsetForEnd - offsetForString - (iLengthStr * 2)); // Address for string
		Pointer pString = this.GetAddr();

		this.SetAddr( this.WriteUnicodeString(name) + PTR(offsetForString));

		//for(int x = -6; x <= sizeFunc; x++) this.WriteByte(0x90, x);

		Pointer pFunc = this.GetAddr();

		//PrintToServer("WriteUnicode = pFunc 0x%X", this.GetAddr());

		this.WriteByte(0x68, 	_, 	MemoryEx_AddAfterWrite);
		this.WriteInt(pString,	_, 	MemoryEx_AddAfterWrite);
		this.WriteWord(0x15FF,	_, 	MemoryEx_AddAfterWrite);
		this.WriteInt(module,	_, 	MemoryEx_AddAfterWrite);
		this.WriteByte(0xC3,	_,	MemoryEx_AddAfterWrite);

		StartPrepSDKCall(SDKCall_Static);
		PrepSDKCall_SetAddress(pFunc);
		PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
		
		Handle h = EndPrepSDKCall();
		Pointer pRes = SDKCall(h);
		
		//PrintToServer("res = 0x%X", iRes);

		this.ChangeSettings(false);
		
		delete h;
		return pRes;
	}	
	Pointer FindPattern(Pointer base, any size, const int[] pattern, int iLength, int iOffset = 0)
	{
		Pointer addr; // default = nullptr
		Pointer end = base + size;

		int iByte;
		bool found;

		while(base < end)
		{
			iByte = LoadFromAddress(base, NumberType_Int8);

			if(iByte == pattern[0])
			{
				found = true;

				for(int x = 1; x < iLength; x++)
				{
					if(pattern[x] != 0x2A)
					{ 
						iByte = LoadFromAddress(base + PTR(x), NumberType_Int8);

						if(iByte != pattern[x])
						{
							found = false;
							break;
						}
					}
				}

				if(found)
				{
					addr = base + PTR(iOffset);
					break;
				}
			}

			base++;
		}

		//char sPattern[32];
		//for(int x = 0; x < iLength; x++) Format(sPattern, sizeof sPattern, "%s 0x%X", sPattern, pattern[x]);

		return addr;
	}
	Pointer GetBaseAddress(const char[] sName)
	{
		CHECKTOINIT(return nullptr)

		ModuleInfo info;
		this.hModules.GetArray(sName, info, sizeof(ModuleInfo));

		return info.base;
	}
	int GetModuleSize(const char[] sName)
	{
		CHECKTOINIT(return -1)

		ModuleInfo info;
		this.hModules.GetArray(sName, info, sizeof(ModuleInfo));
		
		return info.size;
	}

	int ReadByte(int iOffset = 0)
	{
		return LoadFromAddress(this.pAddrBase + PTR(iOffset), NumberType_Int8);
	}
	Pointer WriteByte(any iByte, int iOffset = 0,  int flags = MemoryEx_NoNeedAdd)
	{
		Pointer addr = this.GetAddr() + PTR(iOffset);

		if(this.NeedSave())
		{
			this.lastModif.source = addr;
			this.lastModif.type = ByteType_Byte;
			this.lastModif.original = this.ReadByte(iOffset);
			this.lastModif.current = iByte;

			this.hModifList.PushArray(this.lastModif, sizeof(ModifByte));
		}

		StoreToAddress(addr, iByte, NumberType_Int8);
		
		if(flags & MemoryEx_AddAfterWrite)
		{
			this.Add(0x01);
		}

		return this.GetAddr();
	}
	
	int ReadWord(int iOffset = 0)
	{
		return LoadFromAddress(this.pAddrBase + PTR(iOffset), NumberType_Int16);
	}
	Pointer WriteWord(any iWord, int iOffset = 0,  int flags = MemoryEx_NoNeedAdd)
	{
		Pointer addr = this.GetAddr() + PTR(iOffset);

		if(this.NeedSave())
		{
			this.lastModif.source = addr;
			this.lastModif.type = ByteType_Word;
			this.lastModif.original = LoadFromAddress(addr + PTR(iOffset), NumberType_Int16);
			this.lastModif.current = iWord;

			this.hModifList.PushArray(this.lastModif, sizeof(ModifByte));
		}

		StoreToAddress(addr, iWord, NumberType_Int16);

		if(flags & MemoryEx_AddAfterWrite)
		{
			this.Add(0x02);
		}

		return this.GetAddr();
	}
	
	int ReadInt	(int iOffset = 0)
	{
		return LoadFromAddress(this.pAddrBase + PTR(iOffset), NumberType_Int32);
	}
	Pointer WriteInt(any iNumber, int iOffset = 0, int flags = MemoryEx_NoNeedAdd)
	{
		Pointer addr = this.GetAddr() + PTR(iOffset);

		if(this.NeedSave())
		{
			this.lastModif.source = addr;
			this.lastModif.type = ByteType_Int;
			this.lastModif.original = this.ReadInt(iOffset);
			this.lastModif.current = iNumber;

			this.hModifList.PushArray(this.lastModif, sizeof(ModifByte));
		}

		StoreToAddress(addr, iNumber, NumberType_Int32);

		if(flags & MemoryEx_AddAfterWrite)
		{
			this.Add(0x04);
		}

		return this.GetAddr();
	}
	Pointer WriteData(const int[] data, int iSize)
	{
		int x;

		for(x = 0; x < iSize; x++)
		{
			this.WriteByte(data[x], x);
		}

		return this.GetAddr() + PTR(x);
	}
	void ReadString(char[] sString, int iMaxLength)
	{
		int iByte;

		for(int x = 0; x < iMaxLength; x++)
		{
			iByte = this.ReadByte(x);

			if(iByte == 0x0)
			{
				break;
			}

			sString[x] = iByte;
		}
	}
	Pointer WriteString(const char[] sString, bool bNull = true)
	{
		int x;
		for(x = 0; x < strlen(sString) ; x++)
		{
			this.WriteByte(sString[x], x);
		}

		if(bNull)
		{
			this.WriteByte(0x00, ++x);
		}

		return this.GetAddr() + PTR(x);
	}
	Pointer WriteUnicodeString(const char[] sString, bool bNull = true)
	{
		int x;
		int index;

		Pointer pAddr = this.GetAddr();
		for(x = 0; x <= (strlen(sString) - 1) * 2; x += 2)
		{
			if(this.NeedSave())
			{
				this.lastModif.source = pAddr + PTR(x);
				this.lastModif.type = ByteType_Word;
				this.lastModif.original = this.ReadWord(x);
				this.lastModif.current = sString[index];

				this.hModifList.PushArray(this.lastModif, sizeof(ModifByte));
			}

			this.WriteWord(sString[index], x);
			index++;
		}

		if(bNull)
		{
			x+= 2;
			this.WriteByte(0x00, x);
		}

		return this.GetAddr() + PTR(x);
	}
	OS GetOS()
	{
		CHECKTOINIT(return OS_Unknown)

		return this.os;
	}
	void GetWindowsVersion(int& iMajorVer, int& iMinorVer)
	{
		CHECKTOINIT(return)

		if(this.os != OS_Windows)
		{
			iMajorVer = -1;
			iMinorVer = -1;

			return;
		}
		
		this.SetAddr(0x7FFE0000);
		iMajorVer = this.ReadInt(0x26C);
		iMinorVer = this.ReadInt(0x270);
	}
	Pointer FindString(const char[] sModule, const char[] sString, bool bUnicode = false)
	{
		CHECKTOINIT(return nullptr)


		ModuleInfo info;
		info.base = this.GetBaseAddress(sModule);
		info.size = this.GetModuleSize(sModule);

		if(info.base == nullptr) return nullptr;

		int iLength = strlen(sString);
		int[] pattern = new int[iLength];

		for(int x = 0; x < iLength; x++)
		{
			pattern[x] = sString[x];
		}

		return this.FindPattern(info.base, info.size, pattern, iLength);
	}
	Pointer FindUnicodeString(const char[] sModule, const char[] sString) // very bad
	{
		CHECKTOINIT(return nullptr)

		ModuleInfo info;

		info.base = this.GetBaseAddress(sModule);
		info.size = this.GetModuleSize(sModule);

		if(info.base == nullptr) return nullptr;

		int iLength = strlen(sString) * 2 ;
		int[] pattern = new int[iLength];
		int index;

		for(int x = 0; x < iLength; x += 2)
		{
			pattern[x] = sString[index];
			pattern[x + 1] = 0x00;
			index++;
		}

		return this.FindPattern(info.base, info.size, pattern, iLength);
	}
	Pointer FindValue(const char[] sModule, any iValue, int iNextByte = 0x2A ) // iNextByte - to help
	{
		CHECKTOINIT(return nullptr)

		ModuleInfo info;

		info.base = this.GetBaseAddress(sModule);
		info.size = this.GetModuleSize(sModule);

		if(info.base == nullptr) return nullptr;

		int pattern[8]; // pattern[5] auto to pattern[8];

		pattern[0] = iValue 		& 0xFF;
		pattern[1] = (iValue >> 8) 	& 0xFF;
		pattern[2] = (iValue >> 16) & 0xFF;
		pattern[3] = (iValue >> 24) & 0xFF;
		pattern[4] = iNextByte;

		//PrintToServer("FindValue pattern 0x%X 0x%X 0x%X 0x%X 0x%X", pattern[0], pattern[1], pattern[2], pattern[3], pattern[4] );

		return this.FindPattern(info.base, info.size, pattern, 5);
	}
}

//-----------------------STOCK--------------------------------------
stock int HexToDec(const char[] hex)
{
    int i, result, value;
    while((value = IsHexSymbol(hex[i++])) != -1)
    {
        result = result * 16 + value;
    }
    return result;
}
 
stock int IsHexSymbol(int ch)
{
    if(!ch)
        return -1;
    
    if('0' <= ch && ch <= '9')
        return ch - '0';
    
    ch &= ~0x20;
    if('A' <= ch && ch <= 'F')
        return ch - 'A' + 10;
    
    return -1;
}
