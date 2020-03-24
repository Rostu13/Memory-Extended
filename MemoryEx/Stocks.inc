
#if defined _MemoryEx_Stocks_include_
  #endinput
#endif
#define _MemoryEx_Stocks_include_

stock any ASMSDKCall(Address base)
{
    StartPrepSDKCall(SDKCall_Static);
    PrepSDKCall_SetAddress(base);
    PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
    
    Handle h = EndPrepSDKCall();
    any res = SDKCall(h);

    delete h;
    return res;
}
stock int ReadString(Address base, char[] sResult, int iMaxLength)
{
    int x;
    int iByte;

    for(x = 0; x < iMaxLength; x++)
    {
        iByte = LoadFromAddress(base + view_as<Address>(x), NumberType_Int8);

        if(iByte == 0x0)
        {
            break;
        }

        sResult[x] = iByte;
    }

    return x;
}
stock int ReadUnicodeString(Address base, char[] sResult, int iMaxLength)
{
    int iByte;
    int iOffset;

    for(int x = 0; x < iMaxLength; x+=2)
    {
        iByte = LoadFromAddress(base + view_as<Address>(x), NumberType_Int16);

        if(iByte == 0x0)
        {
            sResult[iOffset] = '\0';
            break;
        }

        sResult[iOffset++] = iByte % 0xFF;
    }

    return iOffset;
}
stock Address FindModuleString(Address base, any size, const char[] sString)
{
    int iLength = strlen(sString);
    int[] pattern = new int[iLength];

    for(int x = 0; x < iLength; x++)
    {
        pattern[x] = sString[x];
    }

    return FindPattern(base, size, pattern, iLength);
}
stock Address FindUnicodeModuleString(Address base, any size, const char[] sString)
{
    int iLength = strlen(sString) * 2 ;
    int[] pattern = new int[iLength];
    int index;

    for(int x = 0; x < iLength; x += 2)
    {
        pattern[x] = sString[index];
        pattern[x + 1] = 0x00;
        index++;
    }

    return FindPattern(base, size, pattern, iLength);
}
stock Address FindValue(Address base, any size, any iValue, const int[] iHelpBytes, int iSize)
{
    int[] pattern = new int[4 + iSize];

    pattern[0] = iValue 		& 0xFF;
    pattern[1] = (iValue >> 8) 	& 0xFF;
    pattern[2] = (iValue >> 16) & 0xFF;
    pattern[3] = (iValue >> 24) & 0xFF;

    for(int x = 0; x < iSize; x++)
    {
        pattern[4 + x] = iHelpBytes[x];
    }

    //PrintToServer("FindValue pattern 0x%X 0x%X 0x%X 0x%X 0x%X", pattern[0], pattern[1], pattern[2], pattern[3], pattern[4] );
    // 0x55 0x66 0x55 = 3
    // 4 = 0x55 5 = 0x66 7 = 0x55

    char sBuffer[128];
    for(int x = 0; x < 4 + iSize; x++)
    {
        Format(sBuffer, sizeof sBuffer, "%s 0x%X", sBuffer, pattern[x]);
    }

    return FindPattern(base, size, pattern, 3 + iSize); // 4 + iSize - 1
}
stock Address FindPattern(Address base, any size, const int[] pattern, int iLength, int iOffset = 0)
{
    Address addr; // default = nullptr
    Address end = base + size;

    int iByte;
    bool found;

    while(base < end)
    {
        iByte = LoadFromAddress(base, NumberType_Int8);

        if(iByte == pattern[0])
        {
            found = true;

            for(int x = 1; x < iLength; x++)
            {
                if(pattern[x] != 0x2A)
                { 
                    iByte = LoadFromAddress(base + view_as<Address>(x), NumberType_Int8);

                    if(iByte != pattern[x])
                    {
                        found = false;
                        break;
                    }
                }
            }

            if(found)
            {
                addr = base + view_as<Address>(iOffset);
                break;
            }
        }

        base++;
    }

    return addr;
}
stock Address FindFreeMemory(Address base, any size, any iLength)
{
    if(!iLength || iLength >= size)
    {
        return Address_Null;
    }

    NumberType type = (iLength % 4 == 0) ? NumberType_Int32 : (iLength % 2 == 0) ? NumberType_Int16 : NumberType_Int8;
    any iOffset = type == NumberType_Int32 ? 4 : type == NumberType_Int16 ?  2 : 1;
    int iCount = iLength / iOffset;

    // iLength = 8
    // iCount = 2
    // iLength = 6
    // iCount = 3

    /*  Example: iOffset = 4.                    ↓ <= base + iSize, but if you try read int [4 byte] => server will crash
        0x00 0x00 0x22 0x50 0x00 0x00 0x00 0x00 0x00 0x?? 0x??
        base + size - iOffset ↓                   ↓ but you lost last byte  
        0x00 0x00 0x22 0x50 0x00 0x00 0x00 0x00 0x00 0x?? 0x??
        base + size - iOffset + 1  ↓ - Now everything is correct and you aren't losing anything :)
        0x00 0x00 0x22 0x50 0x00 0x00 0x00 0x00 0x00 0x?? 0x??

    */

    Address end = base + view_as<Address>(size - iOffset + 1);

    int iByte;
    Address iMove;

    while(end >= base + iLength)
    {
        bool bFind = true;
        for(int x = 0; x < iCount; x++)
        {
            iMove = view_as<Address>(iOffset + 1 - (x * iOffset)); // view_as<Address>(iOffset + 1 - (x * iOffset))
            iByte = LoadFromAddress(end + iMove, type);

            //PrintToServer("adr [0x%X] x [%d] length [%d] offset [%d] count [%d] byte = 0x%X", end + iMove, x, iLength, iOffset, iCount, iByte);

            if(iByte != 0x000000)
            {
                bFind = false;
                break;
            }
        }

        if(bFind)
        {
            return end + iMove;
        }

        end -= iOffset;
    }

    return Address_Null;
}
stock int HexToDec(const char[] hex)
{
    int i, result, value;
    while((value = IsHexSymbol(hex[i++])) != -1)
    {
        result = result * 16 + value;
    }
    return result;
}
 
stock int IsHexSymbol(int ch)
{
    if(!ch)
        return -1;
    
    if('0' <= ch && ch <= '9')
        return ch - '0';
    
    ch &= ~0x20;
    if('A' <= ch && ch <= 'F')
        return ch - 'A' + 10;
    
    return -1;
}