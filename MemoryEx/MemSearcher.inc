
enum struct MEMORY_BASIC_INFORMATION32
{
    Pointer BaseAddress;
    Pointer AllocationBase;
    int AllocationProtect;
    int RegionSize;
    int State;
    int Protect;
    int Type;
}

const int MEM_COMMIT = 0x1000;
const int PAGE_GUARD = 0x100;
const int PAGE_NOACCESS = 0x01;


stock bool IsValidAddress(Address addr, int &numbytes)
{
	return GetServerOS() == OS_Windows ? IsValidAddressWindows(addr, numbytes) : IsValidAddressLinux(addr, numbytes);
}

bool IsValidAddressWindows(Address addr, int &numbytes)
{
	MEMORY_BASIC_INFORMATION32 mbi;
	Address min, max, check = PTR(0x10000); // default lowest memory address
	
	while( VirtualQuery(check, mbi) )
	{
		if( mbi.State == MEM_COMMIT && ((mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) == 0))
		{
			min = mbi.BaseAddress;
			max = mbi.BaseAddress + PTR(mbi.RegionSize - 1);
		
			if( addr >= min && addr <= max )
			{
				if( addr + PTR(numbytes - 1) > max )
				{
					numbytes = view_as<int>(max - addr) + 1;
				}
				return true;
			}
		}
		check += PTR(mbi.RegionSize);
	}
	return false;
}

int VirtualQuery(Address address, MEMORY_BASIC_INFORMATION32 mbi)
{
	static Pointer pFunc;
	static Handle h;

	if( pFunc == Address_Null )
	{
		pFunc = GetProcAddress("kernel32", "VirtualQuery");

		if( pFunc != Address_Null )
		{
			StartPrepSDKCall(SDKCall_Static);
			PrepSDKCall_SetAddress(pFunc);
			PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
			PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
			PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
			PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
			h = EndPrepSDKCall();
		}
	}
	
	int iReturn, iSize = 32;
	Pointer pmbi = VirtualAlloc(iSize);
	if( pmbi )
	{
		iReturn = SDKCall(h, address, pmbi, iSize);
		if( iReturn )
		{
			mbi.BaseAddress 		= LoadFromAddressEx(pmbi, NumberType_Int32);
			mbi.AllocationBase 		= LoadFromAddressEx(pmbi + view_as<Address>(4), NumberType_Int32);
			mbi.AllocationProtect 	= LoadFromAddress(pmbi + view_as<Address>(8), NumberType_Int32);
			mbi.RegionSize 			= LoadFromAddress(pmbi + view_as<Address>(12), NumberType_Int32);
			mbi.State 				= LoadFromAddress(pmbi + view_as<Address>(16), NumberType_Int32);
			mbi.Protect 			= LoadFromAddress(pmbi + view_as<Address>(20), NumberType_Int32);
			mbi.Type 				= LoadFromAddress(pmbi + view_as<Address>(24), NumberType_Int32);
		}
		FreeMemory(pmbi);
	}
	else {
		LogError("Failed to allocate the memory.");
	}
	return iReturn;
}

bool IsValidAddressLinux(Address addr, int &numbytes)
{
	char sBuffer[1024], range[32], access[8], addr_start[16];
	Address min, max;
	int pos;
	bool valid;

	File file = OpenFile("file:///proc/self/maps", "rt"); // thanks to @Rostu
	if( file )
	{
		while( file.ReadLine(sBuffer, sizeof sBuffer) )
		{
			TrimString(sBuffer);
			pos = BreakString(sBuffer, range, sizeof(range));
			if( pos == -1 )
			{
				continue;
			}
			pos = BreakString(sBuffer[pos], access, sizeof(access));
			
			if( access[0] != 'r' )
			{
				continue;
			}
			pos = SplitString(sBuffer, "-", addr_start, sizeof(addr_start));
			
			if( strlen(addr_start) > 8 )
			{
				// 64-bit address is not supported yet
				valid = true;
				break;
			}
			min = PTR(HexToDec(addr_start));
			max = PTR(HexToDec(range[pos]) - 1);
			
			if( addr >= min && addr <= max )
			{
				if( addr + PTR(numbytes - 1) > max )
				{
					numbytes = view_as<int>(max - addr) + 1;
				}
				valid = true;
				break;
			}
		}
		delete file;
	}
	return valid;
}