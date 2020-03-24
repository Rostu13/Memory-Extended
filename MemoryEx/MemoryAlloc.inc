

#include <MemoryEx/ServerLibrary>


stock Address GetAddresAlloc()
{
    static Address base;

    if(base != Address_Null)
    {
        return base;
    }

    Address server = GetServerDLLBase();
    any size = GetServerDLLSize();
    Address pStr = FindModuleString(server, size, "Line %d at \"%s\": %s");

    static int iHelps[] = { 0x50, 0xE8};
    static int iLocal[] = {0x52, 0x8B, 0x08, 0x8B, 0x01, 0xFF};

    Address pFunc = FindValue(server, size, pStr, iHelps, sizeof(iHelps));
    Address pAdr = FindPattern(pFunc, 0x100, iLocal, sizeof(iLocal), -0x04);
    Address adr1 = LoadFromAddress(pAdr, NumberType_Int32); //  ds:g_pMemAllo
    Address adr2 = LoadFromAddress(adr1, NumberType_Int32); // return to tier1 mov     eax, ds:g_pMemAlloc
    Address adr3 = LoadFromAddress(adr2, NumberType_Int32); // mov     ecx, [eax]


    Address adr4 = LoadFromAddress(adr3, NumberType_Int32); // mov     eax, [ecx]
    Address final = LoadFromAddress(adr4 + 0x04, NumberType_Int32); // [eax+4]

   //base = LoadFromAddress(LoadFromAddress(LoadFromAddress(pAdr, NumberType_Int32), NumberType_Int32) + 0x04, NumberType_Int32);

    return adr3;
    PrintToServer("GetAddressAlloc 0x%X pFunc= 0x%X pAdr = 0x%X adr1 = 0x%X  adr2 = 0x%X adr3 = 0x%X adr4 = 0x%X final = 0x%X", pStr, pFunc, pAdr, adr1, adr2, adr3, adr4, final);

}
stock Address VirtualAlloc(int iSize)
{
    StartPrepSDKCall(SDKCall_Static);
    PrepSDKCall_SetAddress(pFunc);
    PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
    
    Handle h = EndPrepSDKCall();
    Pointer pRes = SDKCall(h);

    
    PrintToServer("VirtualAlloc pRes = 0x%X iSize = %d", pRes, iSize);
    
    delete h;
}
stock void ReAllocMemory(Address base, int iSize)
{
    
}
stock void FreeMemory(Address base)
{

}
enum struct MemInfo
{
    Address base;
    int iSize;
}
enum struct MemAlloc
{
    ArrayList mems;

    void Init()
    {
        if(this.mems != null)
        {
            MemInfo info;

            for(int x = 0; x < this.mems.Length; x++)
            {
                this.mems.GetArray(x, info, sizeof(MemInfo));
                this.Free(info.base);
            }

            delete this.mems;
        }

        this.mems = new ArrayList();
    }

    void AllocMemory(int iSize)
    {
        if(iSize < 0)
        {
            return;
        }
    }
    void ReAlloc(Address adr, int iSize)
    {

    }
    void Free(Address adr)
    {

    }
}

stock void AllocMemory()
{

}