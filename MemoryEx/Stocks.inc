
#if defined _MemoryEx_Stocks_include_
  #endinput
#endif
#define _MemoryEx_Stocks_include_

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