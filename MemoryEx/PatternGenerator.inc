#if defined _MemoryEx_Pattern_Generator_include_
  #endinput
#endif
#define _MemoryEx_Pattern_Generator_include_

#define PATTERN_ADD_NULL_CHAR true

enum struct PatternEx
{
    ArrayList pattern;
    
    void Init()
    {
        if(this.pattern != null)
        {
            delete this.pattern;
        }

        this.pattern = new ArrayList();
    }
    void AddString(const char[] sBuffer, bool nullChar = false)
    {
        for(int x = 0; x < strlen(sBuffer); x++)
        {
            this.pattern.Push(sBuffer[x]);
        }

        if(nullChar)
        {
            this.pattern.Push(0x00);
        }
    }
    void AddData(const int[] data, int iSize)
    {
        for(int x = 0; x < iSize; x++)
        {
            this.pattern.Push(data[x]);
        }
    }
    void AddByte(any iValue)
    {
        this.pattern.Push(iValue & 0xFF);
    }
    void AddWord(any iValue)
    {
        this.AddByte(iValue);
        this.pattern.Push((iValue >> 8)     & 0xFF);
    }
    void AddValue(any iValue)
    {
        // this.AddWord(iValue);
        // this.AddWord(iValue >> 16) ... best design code?

        this.pattern.Push(iValue            & 0xFF);
        this.pattern.Push((iValue >> 8)     & 0xFF);
        this.pattern.Push((iValue >> 16)    & 0xFF);
        this.pattern.Push((iValue >> 24)    & 0xFF);
    }
    int GetSize()
    {
        return this.pattern.Length;
    }
    void Generate(int[] pattern, int iSize)
    {
        for(int x = 0; x < iSize; x++)
        {
            pattern[x] = this.pattern.Get(x);
        }
    }
    void Close()
    {
        if(this.pattern != null)
        {
            delete this.pattern;
        }
    }
}
/*

stock int AddStringToPattern(int[] pattern, int startIndex, const char[] sBuffer, bool nullChar = false)
{
    int iLength = strlen(sBuffer);

    for(int x = 0; x < iLength; x++)
    {
        pattern[startIndex + x] = sBuffer[x];
    }

    if(nullChar)
    {
        pattern[startIndex + iLength] = 0x00;
    }

    return startIndex + iLength + 0x01;
}
stock int AddDataToPattern(int[] pattern, int startIndex, const int[] data, int iSize)
{
    for(int x = 0; x < iSize; x++)
	{
		pattern[startIndex + x] = data[x];
	}

    return iSize;
}
stock int AddValueToPattern(int[] pattern, int startIndex, any iValue)
{
	pattern[startIndex] 		= iValue 			& 0xFF;
	pattern[startIndex + 1] 	= (iValue >> 8) 	& 0xFF;
	pattern[startIndex + 2] 	= (iValue >> 16) 	& 0xFF;
	pattern[startIndex + 3] 	= (iValue >> 24) 	& 0xFF;

    return 0x04;
}*/