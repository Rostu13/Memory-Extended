
#if defined _MemoryEx_WindowsFunction_include_
  #endinput
#endif
#define _MemoryEx_WindowsFunction_include_

#include <MemoryEx/Stocks>

stock void WindowsParsePEBHeader(Address base, int& iSize)
{
    int iOffset = LoadFromAddress(base + view_as<Address>(0x3C),            NumberType_Int32); // NT headers start
    iSize       = LoadFromAddress(base + view_as<Address>(iOffset + 0x50),  NumberType_Int32); // NT->OptionalHeader.SizeOfImage
}
stock void GetWindowsVersion(int& iMajorVer, int& iMinorVer)
{
	iMajorVer = LoadFromAddress(view_as<Address>(0x7FFE0000 + 0x26C), NumberType_Int32);
	iMinorVer = LoadFromAddress(view_as<Address>(0x7FFE0000 + 0x270), NumberType_Int32);
}
stock Address GetPEB()
{
	static Address pPEB;

	static int PEBAsm[] =
	{
		0x64, 0xA1, 0x30, 0x00, 0x00, 0x00, // mov eax, dword ptr fs:[30]
		0xC3                                // ret
	};

	if(pPEB != Address_Null)
	{
		return pPEB;
	}
	
	Address pFree = view_as<Address>(0x07FFD000 -  sizeof(PEBAsm)); // hmm... hack?

	for(int x = 0; x < sizeof(PEBAsm); x++)
	{
		StoreToAddress(pFree + view_as<Address>(x), PEBAsm[x], NumberType_Int8);
	}

	pPEB = ASMSDKCall(pFree);

	for(int y = 0;  y < sizeof(PEBAsm); y++) // free memory
	{
		StoreToAddress(pFree + view_as<Address>(y), 0x00, NumberType_Int8);
	}

	return pPEB;
}
stock StringMap WindowsGetDllList(bool bRefresh = false)
{
	static StringMap list;

	if(list == null)
	{
		list = new StringMap();
	}
	else if(bRefresh)
    {
        list.Clear();
    }
	else if(list.Size != 0)
    {
        return list;
    }

	Address PEB = GetPEB();

	if(PEB == Address_Null)
	{
		return list;
	}

	char sLibrary[255];

	Address pLdrData = view_as<Address>(LoadFromAddress(PEB + view_as<Address>(0x0C), NumberType_Int32)); // PPEB_LDR_DATA pldr_data = *(PPEB_LDR_DATA*)(peb + 0x0C);

	Address pLdrCurrent = view_as<Address>(LoadFromAddress(pLdrData + view_as<Address>(0x10), NumberType_Int32) - 0x08); //(PLDR_DATA_TABLE_ENTRY)((UINT)pldr_data->InInitializationOrderModuleList.Blink - 0x08);
	//Address pLdrPrev = LoadFromAddress(pLdrData + 0x0C, NumberType_Int32) - 0x08; // pldr_prev = (PMemoryExLoadedTableEntry)((UINT)pldr_data->InLoadOrderModuleList.Flink - 0x08);
	Address pLdrFirst = pLdrCurrent;

	int iBase;
	int iSize;

	int tmp[2]; // enum struct hack

	do
	{
		iBase = LoadFromAddress(pLdrCurrent + view_as<Address>(0x20), NumberType_Int32);

		if(iBase) // if (pldr_current->DllBase != NULL)
		{
			ReadUnicodeString(view_as<Address>(LoadFromAddress(pLdrCurrent + view_as<Address>(0x30), NumberType_Int32)), sLibrary, sizeof sLibrary); // pldr_current->FullDllName.Buffer
	
			for(int x = strlen(sLibrary) - 1; x >= 0; x--)
			{
				if(sLibrary[x] == '\\') 
				{
					strcopy(sLibrary, sizeof sLibrary, sLibrary[x + 1]);
					break;
				}
				sLibrary[x] = CharToLower(sLibrary[x]);
			}

			WindowsParsePEBHeader(view_as<Address>(iBase), iSize);

			tmp[0] = iBase;
			tmp[1] = iSize;

			list.SetArray(sLibrary, tmp, sizeof(tmp));
		}

		//pLdrPrev = pLdrCurrent;
		pLdrCurrent = view_as<Address>(LoadFromAddress(pLdrCurrent + view_as<Address>(0x0C), NumberType_Int32) - 0x08);
	}
	while(pLdrCurrent != pLdrFirst);

	return list;
}
