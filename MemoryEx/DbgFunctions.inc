#if defined _MemoryEx_Debug_Functions_include_
  #endinput
#endif
#define _MemoryEx_Debug_Functions_include_

#include <MemoryEx/ServerLibrary>

void MemExDGB_RegCommands()
{
    RegServerCmd("sm_mem_ex_loaded_library", Cmd_MemExLoadedLibrary);
}
public Action Cmd_MemExLoadedLibrary(int iClient, int iArgs)
{
    StringMap hList = GetListLibraries();
    StringMapSnapshot hSnap = hList.Snapshot();

    char sName[64];

    ModuleInfo info;

    for(int x = 0; x < hSnap.Length; x++)
    {
        hSnap.GetKey(x, sName, sizeof sName);
        hList.GetArray(sName, info, sizeof(ModuleInfo));
    }
}