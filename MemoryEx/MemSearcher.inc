
//https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-memory_basic_information

enum struct MEMORY_BASIC_INFORMATION
{
    Pointer baseAddress;
    Pointer allocationBase;
    int allocationProtect;
    int partitionId;
    int state;
    int protect;
    int type;
}