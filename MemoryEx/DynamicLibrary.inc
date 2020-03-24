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

		this.InitModule("server");
		this.InitModule("engine");		
	}
	StringMap GetListLibraries()
	{
		return GetServerOS() == OS_Windows ? WindowsGetDllList() : LinuxParseMapsFile();
	}
	Pointer InitModule(const char[] sNameLibrary)
	{
		ModuleInfo info;

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

		StringMap list = this.GetListLibraries();

		if(!list.GetArray(sName, info, sizeof(ModuleInfo)))
		{
			return nullptr;
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
	any GetModuleSize(const char[] sName)
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

		return FindPattern(info.base, info.size, sPattern, iLength, iOffset);
	}
	Pointer FindString(const char[] sModule, const char[] sString)
	{
		ModuleInfo info;
		info.base = this.GetBaseAddress(sModule);
		info.size = this.GetModuleSize(sModule);

		//PrintToServer("DynamicLibrary::findString sModule %s base [0x%X] size [0x%X] str = %s", sModule, info.base, info.size, sString);

		if(info.base == nullptr) return nullptr;

		return FindModuleString(info.base, info.size, sString);
	}
	Pointer FindUnicodeString(const char[] sModule, const char[] sString)
	{
		ModuleInfo info;

		info.base = this.GetBaseAddress(sModule);
		info.size = this.GetModuleSize(sModule);

		if(info.base == nullptr) return nullptr;

		return FindUnicodeModuleString(info.base, info.size, sString);
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

		return FindPattern(info.base, info.size, pattern, 5);
	}
	Pointer FindValueEx(const char[] sModule, any iValue, const int[] iPattern, int iSize)
	{
		ModuleInfo info;

		info.base = this.GetBaseAddress(sModule);
		info.size = this.GetModuleSize(sModule);

		if(info.base == nullptr) return nullptr;

		return FindValue(info.base, info.size, iValue, iPattern, iSize);
	}
}