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

//https://wikipedia.org/wiki/Executable_and_Linkable_Format

static stock bool ParseElfHeader(Address base, int& iSize)
{
    bool isELF = view_as<bool>(LoadFromAddress(base, NumberType_Int32) == 0x464C457F);
    bool x86 = view_as<bool>(LoadFromAddress(base + view_as<Address>(0x04), NumberType_Int8) == 1); // e_ident[EI_CLASS]

    if(!isELF || !x86)
    {
        return false;
    }

    int iFirstPHTable   = LoadFromAddress(base + view_as<Address>(0x1C), NumberType_Int32); // <= e_phoff
    int iPHTableSize    = LoadFromAddress(base + view_as<Address>(0x2A), NumberType_Int16); // <= e_phentsize
    int iCountPHTable   = LoadFromAddress(base + view_as<Address>(0x2C), NumberType_Int16); // <= e_phnum

    Address iStartAddress;

    int iType;
    int iFlags;

    for(int x = 0; x < iCountPHTable; x++)
    {
        iStartAddress = base + view_as<Address>(iFirstPHTable + (iPHTableSize * x));
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