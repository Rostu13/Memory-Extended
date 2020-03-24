#if defined _MemoryEx_Dynamic_Library_include_
  #endinput
#endif
#define _MemoryEx_Dynamic_Library_include_

#include <MemoryEx/ServerLibrary>
#include <MemoryEx/BaseMemory>
#include <MemoryEx/ASM_Instruction>

enum struct ModuleInfo
{
	Pointer base;
	int size;
}

static const char g_sExts[][] = 
{
	"",

	".dll",
	".so",
	".so"
}

enum struct DynamicLibrary
{
    BaseMemory mem;
	StringMap hModules;

	void Init()
	{
		if(this.hModules != null) delete this.hModules;
		this.hModules = new StringMap();

		OS os = GetServerOS();

		ModuleInfo info;

		if(os == OS_Windows)
		{
			info.base = GetServerDLLBase();
			info.size = GetServerDLLSize();

			this.hModules.SetArray("server", info, sizeof(ModuleInfo));
		}
		else if(os == OS_Linux)
		{
			this.InitModule("server");
		}
	}
	Pointer InitModule(const char[] sNameLibrary)
	{
		ModuleInfo info;

		OS os = GetServerOS();

		int iLength = strlen(sNameLibrary);
		int iLengthExts = strlen(g_sExts[os]);
		int iLengthName = iLength + iLengthExts + 1;

		char[] sName = new char[iLengthName ];

		strcopy(sName, iLengthName, sNameLibrary);

		if(iLength > 0x04) // mem.lib.InitModule("engine"); "engine" convert to "engine.so" or "engine.dll"
		{
			if(strcmp(sName[iLength - iLengthExts], g_sExts[os]) != 0)
			{
				FormatEx(sName, iLengthName, "%s%s", sNameLibrary, g_sExts[os]);
			}
		}

		if(os == OS_Windows)
		{
			info.base = this.GetModuleHandle(sName);

			if(info.base == nullptr)
			{
				return nullptr;
			}

			WindowsParsePEBHeader(info.base, info.size);
		}
		else if(os == OS_Linux)
		{
			StringMap list = LinuxParseMapsFile();

			if(!list.GetArray(sName, info, sizeof(ModuleInfo)))
			{
				return nullptr;
			}
		}

		sName[strlen(sName) - iLengthExts] = '\0';
		this.hModules.SetArray(sName, info, sizeof(ModuleInfo));

		return info.base;
	}
	Pointer GetBaseAddress(const char[] sName)
	{
		ModuleInfo info;
		this.hModules.GetArray(sName, info, sizeof(ModuleInfo));

		return info.base;
	}
	int GetModuleSize(const char[] sName)
	{
		ModuleInfo info;
		this.hModules.GetArray(sName, info, sizeof(ModuleInfo));
		
		return info.size;
	}
	Pointer GetEndModule(const char[] sName)
	{
		ModuleInfo info;
		this.hModules.GetArray(sName, info, sizeof(ModuleInfo));

		return (info.base + PTR(info.size));
	}

	Pointer GetModuleHandle(const char[] name)
	{
		OS os = GetServerOS();

		if(os == OS_Windows)
		{
			/*
				push [string address] 	
				call dword ptr [GetModuleHandleW]
				retn							
			*/
			

			Pointer pKernelStr = this.FindUnicodeString("server", "kernel32.dll");
			Pointer module = this.FindValue("server", pKernelStr, 0xFF)  + PTR(0x06);

			Pointer ServerLibBase = GetServerDLLBase();
			int ServerLibSize = GetServerDLLSize();

			if(pKernelStr == nullptr || module == nullptr) 
			{
				ThrowError("GetModuleHandles failed -> Base = 0x%X pKernelStr 0x%X module 0x%X end = 0x%X", ServerLibBase, pKernelStr, module, ServerLibBase + PTR(ServerLibSize) );
				return nullptr;
			}

			module = PTR(LoadFromAddress(module, NumberType_Int32));

			//PrintToServer("GetModuleHandles suss -> Base = 0x%X pKernelStr 0x%X [offset 0x%X] module 0x%X [offset 0x%X] end = 0x%X", g_ServerDLL.base, pKernelStr, pKernelStr - g_ServerDLL.base, module, module - g_ServerDLL.base, g_ServerDLL.base + g_ServerDLL.size );

			static int offsetForString = 0x10; // offset between string and function
			static int offsetForEnd = 0x100; 

			int iLengthStr = strlen(name);

			this.mem.SetAddr((view_as<int>(ServerLibBase) + ServerLibSize) - offsetForEnd - offsetForString - (iLengthStr * 2)); // Address for string
			Pointer pString = name[0] == '0' ? nullptr : this.mem.GetAddr();
			
			this.mem.WriteUnicodeString(name, _, MemoryEx_AddAfterWrite);
			this.mem.Add( offsetForString);

			Pointer pFunc = this.mem.GetAddr();

			ASM.Set(pFunc);

			ASM.Push(pString);
			ASM.Call(module);
			ASM.Retn();

			StartPrepSDKCall(SDKCall_Static);
			PrepSDKCall_SetAddress(pFunc);
			PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
			
			Handle h = EndPrepSDKCall();
			Pointer pRes = SDKCall(h);
			
			//PrintToServer("res = 0x%X", iRes);

			delete h;
			return pRes;
		}
		else if(os == OS_Linux)
		{
			ModuleInfo info;

			if(!this.hModules.GetArray(name, info, sizeof(ModuleInfo)))
			{
				this.InitModule(name);

				if(!this.hModules.GetArray(name, info, sizeof(ModuleInfo)))
				{
					return nullptr;
				}
			}

			return info.base;
		}

		return nullptr;
	}

    Pointer GetProcAddress(const char[] sLibrary, const char[] sName)
	{
		if(GetServerOS() != OS_Windows)
		{
			return nullptr;
		}

		//.text:10228B59 010 68 58 BD 7F 10                          push    offset ProcName ; "InitializeCriticalSectionEx"
		//.text:10228B5E 014 50                                      push    eax             ; hModule
		//.text:10228B5F 018 FF 15 24 B0 78 10                       call    ds:GetProcAddress

		//int iLengthStr = strlen(sName);

		Pointer pStart;
		Pointer pFunc;
		Pointer base = this.GetBaseAddress(sLibrary);

		if(GetServerOS() == OS_Windows)
		{
			Pointer pStr = this.FindString("server", "InitializeCriticalSectionEx");

			pFunc = this.FindValue("server", pStr, 0x50) + PTR(0x07);

			this.mem.SetAddr(pFunc);
			pFunc = PTR(this.mem.ReadInt()); // read address GetProcAddress

			this.mem.SetAddr(this.GetEndModule("server") - PTR(0x250));
			pStr = this.mem.GetAddr();

			this.mem.WriteString(sName, _, MemoryEx_AddAfterWrite);
			this.mem.Add(0x10);

			pStart = this.mem.GetAddr();

			ASM.Set(pStart);

			ASM.Push(pStr);
			ASM.Push(base);

			ASM.Call(pFunc);
			ASM.Retn();
			this.mem.SetAddr(ASM.Get());
		}
		else
		{
			return nullptr;
		}

		StartPrepSDKCall(SDKCall_Static);
		PrepSDKCall_SetAddress(pStart);
		PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
		
		Handle h = EndPrepSDKCall();
		Pointer pRes = SDKCall(h);
		
		//PrintToServer("res pFunc = 0x%X, pStartFunc = 0x%X, proc = 0x%X", pFunc, pStart, pRes);

		
		delete h;
		return pRes;
	}
	
	Pointer FindPattern(const char[] sModule, const int[] sPattern, int iLength, int iOffset = 0)
	{
		ModuleInfo info;

		info.base = this.GetBaseAddress(sModule);
		info.size = this.GetModuleSize(sModule);

		if(info.base == nullptr) return nullptr;

		return this.mem.FindPattern(info.base, info.size, sPattern, iLength);
	}
	Pointer FindString(const char[] sModule, const char[] sString)
	{
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

		return this.mem.FindPattern(info.base, info.size, pattern, iLength);
	}
	Pointer FindUnicodeString(const char[] sModule, const char[] sString)
	{
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

		return this.mem.FindPattern(info.base, info.size, pattern, iLength);
	}
	Pointer FindValue(const char[] sModule, any iValue, int iNextByte = 0x2A ) // iNextByte - to help
	{
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

		return this.mem.FindPattern(info.base, info.size, pattern, 5);
	}
}