/*
	[Extending opportunities for work with memory in SourcePawn]
				Memory Extendвd

	Author: Rostu 				[vk.com/rostu13 | Discord: Rostu#7917]
	Version: 					2.0

	18.12.2019					First release [Windows Only]
	22.12.2019					Version 1.1 [Windows Only]
	19.01.2020					Version 2.0 Windows + linux [x86 only]
*/

#include <MemoryEx/BaseMemory>
#include <MemoryEx/ServerLibrary>
#include <MemoryEx/ASM_Instruction>
#include <MemoryEx/DynamicLibrary>
#include <MemoryEx/CallConvention>

enum struct MemoryEx
{
	BaseMemory mem;
	DynamicLibrary lib;

	void Init()
	{
		this.lib.Init();
	}

}