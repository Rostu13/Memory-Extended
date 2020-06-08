#if defined _MemoryEx_MemoryAlloc_include_
  #endinput
#endif
#define _MemoryEx_MemoryAlloc_include_

#include <MemoryEx/ServerLibrary>

stock Address VirtualAlloc(int iSize)
{
    static Address pFunc;
    static Handle h;

    if(pFunc == Address_Null)
    {
        if(GetServerOS() == OS_Windows)
        {
            pFunc = GetProcAddress("tier0", "g_pMemAlloc");
            if(pFunc != Address_Null)
            {
                pFunc = LoadFromAddressEx(pFunc, NumberType_Int32);
                StartPrepSDKCall(SDKCall_Raw);
                PrepSDKCall_SetVirtual(1);
                PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
                PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
                h = EndPrepSDKCall();
            }
        }
        else
        {
            pFunc = GetImportAddress(NULL_STRING, "malloc");

            StartPrepSDKCall(SDKCall_Static);
            PrepSDKCall_SetAddress(pFunc);
            PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
            PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
            h = EndPrepSDKCall();

        }
    }

    if(GetServerOS() == OS_Windows)
    {
        return SDKCall(h, pFunc, iSize);
    }

    return SDKCall(h, iSize);
}

stock void FreeMemory(Address pBase)
{
    static Address pFunc;
    static Handle h;

    if(pFunc == Address_Null)
    {
        if(GetServerOS() == OS_Windows)
        {
            pFunc = GetProcAddress("tier0", "g_pMemAlloc");
            if(pFunc != Address_Null)
            {
                pFunc = LoadFromAddressEx(pFunc, NumberType_Int32);
                StartPrepSDKCall(SDKCall_Raw);
                PrepSDKCall_SetVirtual(4);
                PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
                h = EndPrepSDKCall();
            }
        }
        else
        {
            pFunc = GetImportAddress(NULL_STRING, "free");

            StartPrepSDKCall(SDKCall_Static);
            PrepSDKCall_SetAddress(pFunc);
            PrepSDKCall_AddParameter(SDKType_PlainOldData, SDKPass_Plain);
            h = EndPrepSDKCall();

        }
    }

    if(GetServerOS() == OS_Windows)
    {
        SDKCall(h, pFunc, pBase);
    }
    else
    {
        SDKCall(h, pBase);
    }
}
