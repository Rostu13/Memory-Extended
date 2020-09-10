/*
	[Extending opportunities for work with memory in SourcePawn]
				Memory Extendвd

	Author: Rostu 				[vk.com/rostu13 | Discord: Rostu#7917]
	Version: 					3.1

	18.12.2019					First release [Windows Only]
	22.12.2019					Version 1.1 [Windows Only]
	19.01.2020					Version 2.0 Windows + linux [x86 only]
	20.03.2020					Version 2.5 [All Source Games]
	05.05.2020					Version 2.7 [Export/Import Tables]
	08.06.2020					Version 3.0 [malloc/free]
	20.08.2020					Version 3.1 [PatternGenerator]
*/

//#define MEMORY_EX_DBG

#include <MemoryEx/ServerLibrary>
#include <MemoryEx/BaseMemory>
#include <MemoryEx/ASM_Instruction>
#include <MemoryEx/DynamicLibrary>
#include <MemoryEx/MemoryAlloc>
//#include <MemoryEx/Smx>

#if defined MEMORY_EX_DBG
#include <MemoryEx/DbgFunctions>
#endif

enum struct MemoryEx
{
	BaseMemory mem;
	DynamicLibrary lib;

	void SetAddr(any address)
	{
		this.mem.SetAddr(address);
	}
	Pointer GetAddr()
	{
		return this.mem.GetAddr();
	}
    void Add(any iOffset)
	{
		this.mem.Add(iOffset);
	}
	int ReadByte(int iOffset = 0)
	{
		return this.mem.ReadByte(iOffset);
	}
	void WriteByte(any iByte, int iOffset = 0,  int flags = MemoryEx_NoNeedAdd)
	{
		this.mem.WriteByte(iByte, iOffset, flags);
	}
	int ReadWord(int iOffset = 0)
	{
		return this.mem.ReadWord(iOffset);
	}
	void WriteWord(any iWord, int iOffset = 0,  int flags = MemoryEx_NoNeedAdd)
	{
		this.mem.WriteWord(iWord, iOffset, flags);
	}
	int ReadInt	(int iOffset = 0)
	{
		return this.mem.ReadInt(iOffset);
	}
	void WriteInt(any iNumber, int iOffset = 0, int flags = MemoryEx_NoNeedAdd)
	{
		this.mem.WriteInt(iNumber, iOffset, flags);
	}
	void WriteData(const int[] data, int iSize, int flags = MemoryEx_NoNeedAdd)
	{
		this.mem.WriteData(data, iSize, flags);
	}
	int ReadString(char[] sString, int iMaxLength)
	{
		return this.mem.ReadString(sString, iMaxLength);
	}
	void WriteString(const char[] sString, bool bNull = true, int flags = MemoryEx_NoNeedAdd)
	{
		this.mem.WriteString(sString, bNull, flags);
	}
	void WriteUnicodeString(const char[] sString, bool bNull = true, int flags = MemoryEx_NoNeedAdd)
	{
		this.mem.WriteUnicodeString(sString, bNull, flags);
	}

	//dynamic library
	StringMap GetListLibraries()
	{
		return this.lib.GetListLibraries();
	}
	Pointer GetModuleHandle(const char[] sName)
	{
		return this.lib.GetModuleHandle(sName);
	}
	any GetModuleSize(const char[] sName)
	{
		return this.lib.GetModuleSize(sName);
	}
	Pointer GetEndModule(const char[] sName)
	{
		return this.lib.GetEndModule(sName);
	}
	Pointer GetProcAddress(const char[] sLibrary, const char[] sName)
	{
		return this.lib.GetProcAddress(sLibrary, sName);
	}
	Pointer GetImportAddress(const char[] sLibrary, const char[] sName)
	{
		return this.lib.GetImportAddress(sLibrary, sName);
	}
	int FindModule(const char[] sModule, char[] sResult, int iMaxLength)
	{
		return this.lib.FindModule(sModule, sResult, iMaxLength);
	}
	Pointer FindPattern(const char[] sModule, const int[] sPattern, int iLength, int iOffset = 0)
	{
		return this.lib.FindPattern(sModule, sPattern, iLength, iOffset);
	}
	Pointer FindString(const char[] sModule, const char[] sString)
	{
		return this.lib.FindString(sModule, sString);
	}
	ArrayList FindAllStrings(const char[] sModule, const char[] sString)
	{
		return this.lib.FindAllStrings(sModule, sString);
	}
	Pointer FindUnicodeString(const char[] sModule, const char[] sString)
	{
		return this.lib.FindUnicodeString(sModule, sString);
	}
	Pointer FindValue(const char[] sModule, any iValue, int iNextByte = 0x2A )
	{
		return this.lib.FindValue(sModule, iValue, iNextByte);
	}
	Pointer FindValueEx(const char[] sModule, any iValue, const int[] iPattern, int iSize)
	{
		return this.lib.FindValueEx(sModule, iValue, iPattern, iSize);
	}
}

stock MemoryEx g_hMem;