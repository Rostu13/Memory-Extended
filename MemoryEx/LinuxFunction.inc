#if defined _MemoryEx_LinuxFunction_include_
  #endinput
#endif

#define _MemoryEx_LinuxFunction_include_

#include <MemoryEx/Stocks>

enum struct LibraryInfo // very bad.  same as in DynamicLibrary.
{
    Address base;
    int size;
}

stock StringMap LinuxParseMapsFile(bool bRefresh = false)
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

    char sBaseAddress[16];
    char sName[64];
    char sBuffer[1024];

    int iLength;
    File file = OpenFile("file:///proc/self/maps", "rt");

    while(file.ReadLine(sBuffer, sizeof sBuffer))
    {
        TrimString(sBuffer);
        iLength = strlen(sBuffer);

        //LogError(sBuffer);

        if(strcmp(sBuffer[iLength - 3], ".so") != 0)
        {
            continue;
        }

        for(int x = iLength - 1; x >= 0; x--)
        {
            if(sBuffer[x] == '/')
            {
                FormatEx(sName, sizeof sName, "%s", sBuffer[x + 1]); // parse name /home/ttt/serverfiles/csgo/addons/sourcemod/extensions/dbi.sqlite.ext.so => sName = dbi.sqlite.ext.so

                //fix name conflict
                // /home/ttt/serverfiles/csgo/addons/metamod/bin/server.so
                // /home/ttt/serverfiles/csgo/bin/server.so

                if(!strcmp(sName, "server.so")) 
                {
                    if(StrContains(sBuffer, "/metamod/") != -1) // it's metamod
                    {
                        Format(sName, sizeof sName, "mm_%s", sName);
                    }
                }
                break;
            }
        }

        LibraryInfo info;

        if(list.GetArray(sName, info, sizeof(LibraryInfo))) // exist
        {
            continue;
        }

        FormatEx(sBaseAddress, 9, "%s", sBuffer);
        info.base = view_as<Address>(HexToDec(sBaseAddress));
        
        if(ParseElfHeader(info.base, info.size))
        {
            list.SetArray(sName, info, sizeof(LibraryInfo));
        }
    }

    delete file;
    return list;
}

stock Address GetFirstElfTable(Address base)
{
    return view_as<Address>(LoadFromAddress(base + view_as<Address>(0x1C), NumberType_Int32)); // <= e_phoff;
}
stock int GetElfTableSize(Address base)
{
    return LoadFromAddress(base + view_as<Address>(0x2A), NumberType_Int16); // <= e_phentsize
}
stock int GetCountElfTable(Address base)
{
    return LoadFromAddress(base + view_as<Address>(0x2C), NumberType_Int16); // <= e_phnum;
}
//https://wikipedia.org/wiki/Executable_and_Linkable_Format
static stock bool ParseElfHeader(Address base, int& iSize)
{
    bool isELF = view_as<bool>(LoadFromAddress(base, NumberType_Int32) == 0x464C457F);
    bool x86 = view_as<bool>(LoadFromAddress(base + view_as<Address>(0x04), NumberType_Int8) == 1); // e_ident[EI_CLASS]

    if(!isELF || !x86)
    {
        return false;
    }

    Address pFirstPHTable   = LoadFromAddressEx(base + view_as<Address>(0x1C), NumberType_Int32); // <= e_phoff
    int iPHTableSize        = GetElfTableSize(base);
    int iCountPHTable       = GetCountElfTable(base);

    Address iStartAddress;

    int iType;
    int iFlags;

    for(int x = 0; x < iCountPHTable; x++)
    {
        iStartAddress = base + pFirstPHTable + view_as<Address>(iPHTableSize * x);
        iType = LoadFromAddress(iStartAddress, NumberType_Int32);

        if(iType != 1) // 1 == PT_LOAD
        {
            continue;
        }

        iFlags = LoadFromAddress(iStartAddress + view_as<Address>(0x18), NumberType_Int32); // p_flags

        if(iFlags == (0x01|0x04)) // hdr.p_flags == (PF_X|PF_R)
        {
            iSize = LoadFromAddress(iStartAddress + view_as<Address>(0x10), NumberType_Int32) - 1; // p_filesz
            break;
        }
    }

    return true;
    
}
stock Address GetProcAddressLinux(Address pBase, const char[] sName )
{
     //From MemoryEx/LinuxFunction/ParseElfHeader
    Address pFirstTable = GetFirstElfTable(pBase);
    int iPHTableSize    = GetElfTableSize(pBase);
    int iCountPHTable   = GetCountElfTable(pBase);

    Address iStartAddress;
    Address pDynamic;

    int iType;

    for(int x = 0; x < iCountPHTable; x++)
    {
        iStartAddress = pBase + pFirstTable + view_as<Address>(iPHTableSize * x);
        iType = LoadFromAddress(iStartAddress, NumberType_Int32);

        if(iType != 2) // 2 == PT_DYNAMIC
        {
            continue;
        }

        pDynamic = pBase + LoadFromAddressEx(iStartAddress + view_as<Address>(0x08), NumberType_Int32); // p_vaddr
        break;
    }

    //DumpOnAddress(pDynamic, 0xE8, 8);

    Address offset;
    Address pStrTab;
    Address pSymTab;
    Address pStr;
    Address pFinal;

    while((iType = LoadFromAddress(pDynamic + offset, NumberType_Int32)) != 0)
    {
        switch(iType)
        {
            case 0x05: pStrTab  = LoadFromAddressEx(pDynamic + offset + view_as<Address>(0x04), NumberType_Int32);
            case 0x06: pSymTab  = LoadFromAddressEx(pDynamic + offset + view_as<Address>(0x04), NumberType_Int32); 
        }
        offset += view_as<Address>(0x08);
    }

    char sStr[256];

    int iSize = view_as<int>(pStrTab - pSymTab) / 0x10;
    int iLen = strlen(sName);
    int iLenStr;

    //DumpOnAddress(pSymTab, 0x100, 16);

    for(int x = 0; x < iSize; x++)
    {
        pStr = pStrTab + LoadFromAddressEx(pSymTab, NumberType_Int32);
        iLenStr = ReadString(pStr, sStr, sizeof sStr);

        if(iLen == iLenStr)
        {
            if(!strcmp(sName, sStr))
            {
                pFinal = pBase + LoadFromAddressEx(pSymTab + view_as<Address>(0x04), NumberType_Int32);
                
                if(pFinal == pBase)
                {
                    return Address_Null;
                }

                return pFinal
            }
        }

        pSymTab += view_as<Address>(0x10);
    }

    return Address_Null;
}
stock Address GetImportAddressLinux(Address pBase, const char[] sName )
{
    StringMap hList = LinuxParseMapsFile();
    StringMapSnapshot hSnap = hList.Snapshot();

    Address pAddress;

    char sLibrary[64];

    LibraryInfo info;

    for(int x = 0; x < hSnap.Length; x++)
    {
        hSnap.GetKey(x, sLibrary, sizeof sLibrary);
        hList.GetArray(sLibrary, info, sizeof(LibraryInfo));

        pAddress = GetProcAddressLinux(info.base, sName);

        if(pAddress != Address_Null && pAddress != info.base)
        {
            delete hSnap;
            return pAddress;
        }
    }

    delete hSnap;
    return Address_Null;
}