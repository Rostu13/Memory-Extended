/*
	[Extending opportunities for work with memory in SourcePawn]
				Memory Extendвd

	Author: Rostu 				[vk.com/rostu13 | Discord: Rostu#7917]
	Version: 					2.5

	18.12.2019					First release [Windows Only]
	22.12.2019					Version 1.1 [Windows Only]
	19.01.2020					Version 2.0 Windows + linux [x86 only]
	20.03.2020					Version 2.5 [All Source Games]
*/

#include <MemoryEx/ServerLibrary>
#include <MemoryEx/BaseMemory>
#include <MemoryEx/ASM_Instruction>
#include <MemoryEx/DynamicLibrary>
//dev
enum struct MemoryEx
{
	BaseMemory mem;
	DynamicLibrary lib;

	void Init()
	{
		this.lib.Init();
	}
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
	Pointer InitModule(const char[] sNameLibrary)
	{
		return this.lib.InitModule(sNameLibrary);
	}
	Pointer GetBaseAddress(const char[] sName)
	{
		return this.lib.GetBaseAddress(sName);
	}
	any GetModuleSize(const char[] sName)
	{
		return this.lib.GetModuleSize(sName);
	}
	Pointer GetEndModule(const char[] sName)
	{
		return this.lib.GetEndModule(sName);
	}
	Pointer GetModuleHandle(const char[] name)
	{
		return this.lib.GetModuleHandle(name);
	}
	Pointer GetProcAddress(const char[] sLibrary, const char[] sName)
	{
		return this.lib.GetProcAddress(sLibrary, sName);
	}
	Pointer FindPattern(const char[] sModule, const int[] sPattern, int iLength, int iOffset = 0)
	{
		return this.lib.FindPattern(sModule, sPattern, iLength, iOffset);
	}
	Pointer FindString(const char[] sModule, const char[] sString)
	{
		return this.lib.FindString(sModule, sString);
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