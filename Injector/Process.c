#include "Stdafx.h"



PTSTR __cdecl FormatString(PTSTR ptszFormat, ...);



BOOL UpdateProcessList(HWND hList)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }
    
    SendMessage(hList, LB_RESETCONTENT, NULL, NULL);

    PROCESSENTRY32 ProcEntry;
    ProcEntry.dwSize = sizeof(PROCESSENTRY32);
    Process32First(hSnapshot, &ProcEntry);      //Ignore system process...

    while (Process32Next(hSnapshot, &ProcEntry))
    {
        if (!(_tcscmp(ProcEntry.szExeFile, __TEXT("System"))))
        {
            continue;
        }

        PTSTR ptszCurLine = FormatString(__TEXT("[%i] %s"), ProcEntry.th32ProcessID, ProcEntry.szExeFile);
        if (!ptszCurLine)
        {
            return FALSE;
        }

        SendMessage(hList, LB_ADDSTRING, NULL, ptszCurLine);

        if (ptszCurLine)
        {
            free(ptszCurLine);
        }
    }

    return TRUE;
}



BOOL UpdateModuleList(HWND hList, DWORD dwPid)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    SendMessage(hList, LB_RESETCONTENT, NULL, NULL);

    MODULEENTRY32 ModEntry;
    ModEntry.dwSize = sizeof(MODULEENTRY32);
    Module32First(hSnapshot, &ModEntry);

    while (Module32Next(hSnapshot, &ModEntry))
    {
        PTSTR ptszCurLine = FormatString(__TEXT("[%p] %s"), ModEntry.modBaseAddr, ModEntry.szModule);
        if (!ptszCurLine)
        {
            return FALSE;
        }

        SendMessage(hList, LB_ADDSTRING, NULL, ptszCurLine);

        if (ptszCurLine)
        {
            free(ptszCurLine);
        }
    }

    return TRUE;
}



PTSTR __cdecl FormatString(PTSTR ptszFormat, ...)
{
    va_list pArgs       = NULL;
    PTSTR   ptszText    = NULL;

    __crt_va_start(pArgs, ptszFormat);
    __try
    {
        int iTextLength = _vsctprintf(ptszFormat, pArgs) + 1;
        if (iTextLength == 1)
        {
            __leave;
        }

        ptszText = malloc(iTextLength * sizeof(TCHAR));
        if (!ptszText)
        {
            __leave;
        }

        _vstprintf_s(ptszText, iTextLength, ptszFormat, pArgs);
    }
    __finally
    {
        __crt_va_end(pArgs, ptszFormat);
    }

    return ptszText;
}