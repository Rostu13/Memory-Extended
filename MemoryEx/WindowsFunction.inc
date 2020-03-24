
#if defined _MemoryEx_WindowsFunction_include_
  #endinput
#endif
#define _MemoryEx_WindowsFunction_include_

#include <MemoryEx/ServerLibrary>

stock void WindowsParsePEBHeader(Address base, int& iSize)
{
    int iOffset = LoadFromAddress(base + view_as<Address>(0x3C),            NumberType_Int32); // NT headers start
    iSize       = LoadFromAddress(base + view_as<Address>(iOffset + 0x50),  NumberType_Int32); // NT->OptionalHeader.SizeOfImage
}
stock void GetWindowsVersion(int& iMajorVer, int& iMinorVer)
{
	//if(GetServerOS() != OS_Windows)
	//{
	//	iMajorVer = -1;
	//	iMinorVer = -1;

		//return;
	//}

	iMajorVer = LoadFromAddress(view_as<Address>(0x7FFE0000 + 0x26C), NumberType_Int32);
	iMinorVer = LoadFromAddress(view_as<Address>(0x7FFE0000 + 0x270), NumberType_Int32);
}
