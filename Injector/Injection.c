#include "Stdafx.h"

#include "InjectionMacros.h"



BOOL LoadLibraryInject(DWORD dwProcessId, PTSTR ptszDllPath, PTSTR ptszDll);



void Inject(PTSTR ptszProcess, PTSTR ptszDllPath, PTSTR ptszDll, BYTE bInjectionFlag)
{
    TCHAR   ptszProcessId[32];
    BOOL    bFoundOpen      = FALSE;
    BOOL    bFoundClose     = FALSE;
    int     iStartIndex     = -1;
    for (int i = 0, j = 0; i < _tcslen(ptszProcess); i++)
    {
        if (ptszProcess[i] == __TEXT('['))
        {
            bFoundOpen = TRUE;
        }
        else if (ptszProcess[i] == __TEXT(']'))
        {
            ptszProcessId[j]    = __TEXT('\0');
            bFoundClose         = TRUE;
        }
        else
        {
            ptszProcessId[j] = ptszProcess[i];
            j++;
        }

        if (bFoundOpen && bFoundClose)
        {
            iStartIndex = i + 2;
            break;
        }
    }
    DWORD dwProcessId = _ttoi(ptszProcessId);
    TCHAR ptszFormattedProcess[MAX_PATH];       //Pid section removed
    _tcscpy_s(ptszFormattedProcess, MAX_PATH, ptszProcess + iStartIndex);
    
    switch (bInjectionFlag)
    {
    
    case LOADLIBRARY_INJECTION_ID:
    {
        if (!LoadLibraryInject(dwProcessId, ptszDllPath, ptszDll))
        {
            MessageBox(NULL, __TEXT("Injection failed."), __TEXT(":("), MB_OK);
        }
        else
        {
            MessageBox(NULL, __TEXT("Injection success."), __TEXT(":)"), MB_OK);
        }

        break;
    }

    }
}



BOOL LoadLibraryInject(DWORD dwProcessId, PTSTR ptszDllPath, PTSTR ptszDll)
{
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
    if (!hProc)
    {
        return FALSE;
    }
    __try
    {
        PVOID pTargetMem = VirtualAllocEx(hProc, NULL, 1 << 12, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (!pTargetMem)
        {
            return FALSE;
        }

        if (!WriteProcessMemory(hProc, pTargetMem, ptszDllPath, (_tcslen(ptszDllPath) + 1) * sizeof(TCHAR), NULL))
        {
            return FALSE;
        }

        if (!CreateRemoteThread(hProc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibrary, pTargetMem, NULL, NULL))
        {
            return FALSE;
        }
    }
    __finally
    {
        if (hProc)
        {
            CloseHandle(hProc);
        }
    }

    return TRUE;
}