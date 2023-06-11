#include "Stdafx.h"



void UnloadModule(DWORD dwPid, PVOID pBaseAddr)
{
    if (!pBaseAddr || !dwPid)
    {
        return;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
    if (!hProcess)
    {
        return;
    }
    __try
    {
        CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)FreeLibrary, pBaseAddr, NULL, NULL);
    }
    __finally
    {
        if (hProcess)
        {
            CloseHandle(hProcess);
        }
    }
}