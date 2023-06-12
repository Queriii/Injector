#include "Stdafx.h"

#include "resource.h"
#include "MainMacros.h"
#include "InjectionMacros.h"
#include "DimensionMacros.h"
#include "Process.h"
#include "Injection.h"
#include "Unloading.h"



        LRESULT CALLBACK    WndMainProc             (HWND hWindow, UINT uiMessage, WPARAM uiParam, LPARAM lParam);
        LRESULT CALLBACK    WndSubsectionProc       (HWND hWindow, UINT uiMessage, WPARAM uiParam, LPARAM lParam);
inline  void                SetWndClass             (WNDCLASS* WndClass, UINT uiStyle, WNDPROC pfnWndProc, int cbClsExtra, int cbWndExtra, HINSTANCE hInstance, HICON hIcon,                                                       HCURSOR hCursor, HBRUSH hBackground, PTSTR ptszMenuName, PTSTR ptszClassName);   
        INT_PTR             ProcessSelectionDialog  (HWND hDlg, UINT uiMessage, WPARAM uiParam, LPARAM lParam);
        INT_PTR             ConfigDialog            (HWND hDlg, UINT uiMessage, WPARAM uiParam, LPARAM lParam);
        INT_PTR             ModuleDialog            (HWND hDlg, UINT uiMessage, WPARAM uiParmam, LPARAM lParam);




int WINAPI WIN_MAIN(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrev, _In_ PTSTR ptszCommandArgs, _In_ int iWindowState)
{
    WNDCLASS MainWndClass;
    SetWndClass(&MainWndClass, NULL, WndMainProc, 0, 0, hInstance, LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW), GetStockObject(BLACK_BRUSH), NULL,                       MAIN_WNDCLASS_NAME);
    if (!RegisterClass(&MainWndClass))
    {
        return -1;
    }

    WNDCLASS SubsectionWndClass;
    SetWndClass(&SubsectionWndClass, NULL, WndSubsectionProc, 0, 0, hInstance, NULL, LoadCursor(NULL, IDC_ARROW), GetStockObject(WHITE_BRUSH), NULL, SUBSECTION_WNDCLASS_NAME);
    if (!RegisterClass(&SubsectionWndClass))
    {
        return -1;
    }


    HWND hWindow = CreateWindow(MAIN_WNDCLASS_NAME, MAIN_WND_NAME, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT, MAIN_WND_WIDTH,                                  MAIN_WND_HEIGHT, NULL, LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_MAIN)), hInstance, NULL);
    if (!hWindow)
    {
        return -1;
    }

    ShowWindow(hWindow, iWindowState);
    UpdateWindow(hWindow);

    MSG Msg;
    for (;;)
    {
        BOOL bStatus = GetMessage(&Msg, NULL, NULL, NULL);
        if (bStatus == -1)
        {
            return -1;
        }
        else if (!bStatus)
        {
            break;
        }
        else
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
    }

    return Msg.wParam;
}



typedef struct WndMainProcInfo
{
    HMENU   hMenu;

    int     cxClientWidth;
    int     cyClientHeight;

    HWND    hProcSubsection;                    //Choose target proc...
    HWND    hFileSubsection;                    //Choose target file...

    BYTE    bInjectionFlag;                     //1 = LoadLibrary (Highest valid)
    TCHAR   ptszSelectedProcess [MAX_PATH];
    TCHAR   ptszSelectedDllPath [MAX_PATH];
    TCHAR   ptszSelectedDll     [MAX_PATH];
}WndMainProcInfo, * PWndMainProcInfo;
BOOL                ValidateInjectionAttempt(PWndMainProcInfo pSettings);
LRESULT CALLBACK    WndMainProc             (HWND hWindow, UINT uiMessage, WPARAM uiParam, LPARAM lParam)
{
    static WndMainProcInfo Shared;

    switch (uiMessage)
    {

    case WM_CREATE:
    {
        Shared.hMenu = ((CREATESTRUCT*)lParam)->hMenu;

        Shared.hProcSubsection          = CreateWindow(SUBSECTION_WNDCLASS_NAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWindow, SUBSECTION_PROC_ID,                                                                      ((CREATESTRUCT*)lParam)->hInstance, NULL);
        if (!Shared.hProcSubsection)
        {
            SendMessage(hWindow, WM_CLOSE, NULL, NULL);
            return 0;
        }

        Shared.hFileSubsection          = CreateWindow(SUBSECTION_WNDCLASS_NAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWindow, SUBSECTION_FILE_ID,                                                                      ((CREATESTRUCT*)lParam)->hInstance, NULL);
        if (!Shared.hFileSubsection)
        {
            SendMessage(hWindow, WM_CLOSE, NULL, NULL);
            return 0;
        }

        Shared.bInjectionFlag           = LOADLIBRARY_INJECTION_ID;     //Default injection type
        Shared.ptszSelectedDll[0]       = NULL;
        Shared.ptszSelectedDllPath[0]   = NULL;
        Shared.ptszSelectedProcess[0]   = NULL;

        return 0;
    }

    case WM_SIZE:
    {
        Shared.cxClientWidth    = LOWORD(lParam);
        Shared.cyClientHeight   = HIWORD(lParam);

        MoveWindow(Shared.hProcSubsection, 0, 0, Shared.cxClientWidth, Shared.cyClientHeight / 2, TRUE);
        MoveWindow(Shared.hFileSubsection, 0, (Shared.cyClientHeight / 2) + 1, Shared.cxClientWidth, (Shared.cyClientHeight / 2), TRUE);

        return 0;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(uiParam))
        {

        case IDM_SETTINGS_CONFIG:
        {
            DialogBoxParam(GetWindowLongPtr(hWindow, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_CONFIG), hWindow, ConfigDialog, &(Shared.bInjectionFlag));
            break;
        }

        case IDM_SETTINGS_UNLOADMODULES:
        {
            DialogBoxParam(GetWindowLongPtr(hWindow, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_MODULESELECT), hWindow, ModuleDialog, &Shared);
            break;
        }

        }

        return 0;
    }

    case WM_KEYDOWN:
    {
        switch (uiParam)
        {

        case VK_END:
        {
            if (!ValidateInjectionAttempt(&Shared))
            {
                MessageBox(NULL, __TEXT("Invalid Injection Attempt"), __TEXT(":("), MB_OK);
                break;
            }

            Inject(Shared.ptszSelectedProcess, Shared.ptszSelectedDllPath, Shared.ptszSelectedDll, Shared.bInjectionFlag);

            break;
        }

        default:
        {
            return (DefWindowProc(hWindow, uiMessage, uiParam, lParam));
        }

        }

        return 0;
    }

    case QM_PROCUPDATE:
    {
        _tcscpy_s(Shared.ptszSelectedProcess, MAX_PATH, lParam);
        EnableMenuItem(Shared.hMenu, IDM_SETTINGS_UNLOADMODULES, MF_ENABLED);
        return 0;
    }

    case QM_FILEUPDATE:
    {
        _tcscpy_s(Shared.ptszSelectedDllPath, MAX_PATH, uiParam);
        _tcscpy_s(Shared.ptszSelectedDll, MAX_PATH, lParam);
        return 0;
    }

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }

    default:
    {
        return (DefWindowProc(hWindow, uiMessage, uiParam, lParam));
    }

    }
}



typedef struct WndSubsectionProcInfo
{
    int     cxButtonWidth;
    int     cyButtonHeight;
    HWND    hProcButton;
    HWND    hFileButton;

    int     cxClientWidth;
    int     cyClientHeight;

    TCHAR   ptszSelectedProcess [MAX_PATH];
    TCHAR   ptszSelectedDllPath [MAX_PATH];
    TCHAR   ptszSelectedDll     [MAX_PATH];

    int     cxDisplayWidth;
    int     cyDisplayHeight;
    HWND    hProcDisplay;
    HWND    hFileDisplay;
}WndSubsectionProcInfo;
LRESULT CALLBACK WndSubsectionProc(HWND hWindow, UINT uiMessage, WPARAM uiParam, LPARAM lParam)
{
    static WndSubsectionProcInfo Shared;

    switch (uiMessage)
    {
    
    case WM_CREATE:
    {
        switch (GetWindowLongPtr(hWindow, GWLP_ID))
        {
        
        case SUBSECTION_PROC_ID:
        {
            Shared.hProcButton              = CreateWindow(__TEXT("button"), __TEXT("Select Process"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWindow,                                                           SUBSECTION_PROC_BUTTON_ID, ((CREATESTRUCT*)lParam)->hInstance, NULL);
            Shared.hProcDisplay             = CreateWindow(__TEXT("static"), NULL, WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 0, 0, 0, hWindow, SUBSECTION_PROC_DISPLAY_ID,                                                      ((CREATESTRUCT*)lParam)->hInstance, NULL);
            Shared.ptszSelectedProcess[0]   = __TEXT('\0');
            break;
        }

        case SUBSECTION_FILE_ID:
        {
            Shared.hFileButton              = CreateWindow(__TEXT("button"), __TEXT("Select File"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 0, 0, 0, hWindow,                                                              SUBSECTION_FILE_BUTTON_ID, ((CREATESTRUCT*)lParam)->hInstance, NULL);
            Shared.hFileDisplay             = CreateWindow(__TEXT("static"), NULL, WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 0, 0, 0, hWindow, SUBSECTION_FILE_DISPLAY_ID,                                                      ((CREATESTRUCT*)lParam)->hInstance, NULL);
            Shared.ptszSelectedDll[0]       = __TEXT('\0');
            Shared.ptszSelectedDllPath[0]   = __TEXT('\0');
            break;
        }

        }

        Shared.cxButtonWidth    = max(_tcslen(__TEXT("Select Process")), _tcslen(__TEXT("Select File"))) * LOWORD(GetDialogBaseUnits());
        Shared.cyButtonHeight   = (7.0 / 4.0) * HIWORD(GetDialogBaseUnits());

        return 0;
    }

    case WM_SIZE:
    {
        Shared.cxClientWidth    = LOWORD(lParam);
        Shared.cyClientHeight   = HIWORD(lParam);

        Shared.cxDisplayWidth   = Shared.cxClientWidth / 2;
        Shared.cyDisplayHeight  = HIWORD(GetDialogBaseUnits());

        switch (GetWindowLongPtr(hWindow, GWLP_ID))
        {

        case SUBSECTION_PROC_ID:
        {   
            MoveWindow(Shared.hProcButton, 10, (Shared.cyClientHeight / 2) - (Shared.cyButtonHeight / 2), Shared.cxButtonWidth, Shared.cyButtonHeight, TRUE);
            MoveWindow(Shared.hProcDisplay, Shared.cxClientWidth / 2, (Shared.cyClientHeight / 2) - (Shared.cyDisplayHeight / 2), Shared.cxDisplayWidth, Shared.cyDisplayHeight,         TRUE);
            break;
        }

        case SUBSECTION_FILE_ID:
        { 
            MoveWindow(Shared.hFileButton, 10, (Shared.cyClientHeight / 2) - (Shared.cyButtonHeight / 2), Shared.cxButtonWidth, Shared.cyButtonHeight, TRUE);
            MoveWindow(Shared.hFileDisplay, Shared.cxClientWidth / 2, (Shared.cyClientHeight / 2) - (Shared.cyDisplayHeight / 2), Shared.cxDisplayWidth, Shared.cyDisplayHeight,         TRUE);
            break;
        }

        }

        return 0;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(uiParam))
        {

        case SUBSECTION_PROC_BUTTON_ID:
        {
            if (HIWORD(uiParam) == BN_CLICKED)
            {
                if (DialogBoxParam((GetWindowLongPtr(hWindow, GWLP_HINSTANCE)), MAKEINTRESOURCE(IDD_PROCESS_SELECT), hWindow, ProcessSelectionDialog, Shared.ptszSelectedProcess))
                {
                    SetWindowText(Shared.hProcDisplay, Shared.ptszSelectedProcess);
                    SendMessage(GetParent(hWindow), QM_PROCUPDATE, NULL, Shared.ptszSelectedProcess);
                }
            }

            SetFocus(GetParent(hWindow));
            break;
        }

        case SUBSECTION_FILE_BUTTON_ID:
        {
            if (HIWORD(uiParam) == BN_CLICKED)
            {

                OPENFILENAME Ofn;
                Ofn.lStructSize         = sizeof(OPENFILENAME);
                Ofn.hwndOwner           = hWindow;
                Ofn.hInstance           = NULL;
                Ofn.lpstrFilter         = __TEXT("Dll Files (*.DLL)\0*.dll\0\0");
                Ofn.lpstrCustomFilter   = NULL;
                Ofn.nMaxCustFilter      = 0;
                Ofn.nFilterIndex        = 0;
                Ofn.lpstrFile           = Shared.ptszSelectedDllPath;
                Ofn.nMaxFile            = MAX_PATH;
                Ofn.lpstrFileTitle      = Shared.ptszSelectedDll;
                Ofn.nMaxFileTitle       = MAX_PATH;
                Ofn.lpstrInitialDir     = NULL;
                Ofn.lpstrTitle          = __TEXT("Select Dll...");
                Ofn.Flags               = NULL;
                Ofn.nFileOffset         = NULL;
                Ofn.nFileExtension      = NULL;
                Ofn.lpstrDefExt         = __TEXT("dll");
                Ofn.lCustData           = 0L;
                Ofn.lpfnHook            = NULL;
                Ofn.lpTemplateName      = NULL;

                if (GetOpenFileName(&Ofn))
                {
                    SetWindowText(Shared.hFileDisplay, Shared.ptszSelectedDll);
                    SendMessage(GetParent(hWindow), QM_FILEUPDATE, Shared.ptszSelectedDllPath, Shared.ptszSelectedDll);
                }

                SetFocus(GetParent(hWindow));
            }

            break;
        }

        default:
        {
            return (DefWindowProc(hWindow, uiMessage, uiParam, lParam));
        }

        }

        return 0;
    }

    case WM_CTLCOLORSTATIC:
    {
        return (GetStockObject(WHITE_BRUSH));
    }

    default:
    {
        return (DefWindowProc(hWindow, uiMessage, uiParam, lParam));
    }

    }
}



typedef struct ProcessSelectionDialogInfo
{
    HWND    hList;
    TCHAR*  ptszProcess;
    BYTE    bCloseFlag;
}ProcessSelectionDialogInfo;
INT_PTR ProcessSelectionDialog(HWND hDlg, UINT uiMessage, WPARAM uiParam, LPARAM lParam)
{
    static ProcessSelectionDialogInfo Shared;

    switch (uiMessage)
    {
    
    case WM_INITDIALOG:
    {
        Shared.bCloseFlag   = 0;
        Shared.ptszProcess  = lParam;

        Shared.hList        = GetDlgItem(hDlg, IDC_LIST_PROCESS);
        UpdateProcessList(Shared.hList);
        
        SetTimer(hDlg, SUBSECTION_PROC_TIMER_ID, 5000, NULL);

        return TRUE;
    }

    case WM_TIMER:
    {
        int iOldSelection   = SendMessage(Shared.hList, LB_GETCURSEL, NULL, NULL);
        int iOldTop         = SendMessage(Shared.hList, LB_GETTOPINDEX, NULL, NULL);
        
        SendMessage(Shared.hList, WM_SETREDRAW, FALSE, 0);
        UpdateProcessList(Shared.hList);
        if (iOldSelection != LB_ERR)
        {
            SendMessage(Shared.hList, LB_SETCURSEL, iOldSelection, NULL);
        }
        SendMessage(Shared.hList, LB_SETTOPINDEX, iOldTop, NULL);
        SendMessage(Shared.hList, WM_SETREDRAW, TRUE, 0);

        return TRUE;
    }

    case WM_COMMAND:
    {
        switch (HIWORD(uiParam))
        {
        
        case LBN_DBLCLK:
        {
            if (SendMessage(Shared.hList, LB_GETTEXT, SendMessage(Shared.hList, LB_GETCURSEL, NULL, NULL), Shared.ptszProcess) != LB_ERR)
            {
                int iTest = 100;
                Shared.bCloseFlag = 1;
            }

            SendMessage(hDlg, WM_CLOSE, NULL, NULL);
            break;
        }

        }

        return TRUE;
    }

    case WM_CLOSE:
    {
        EndDialog(hDlg, Shared.bCloseFlag);
        return TRUE;
    }

    }

    return FALSE;
}



typedef struct ConfigDialogInfo
{
    BYTE    bLocalSaveInjection;
    BYTE*   pSaveInjection;
}ConfigDialogInfo;
INT_PTR ConfigDialog(HWND hDlg, UINT uiMessage, WPARAM uiParam, LPARAM lParam)
{
    static ConfigDialogInfo Shared;

    switch (uiMessage)
    {

    case WM_INITDIALOG:
    {
        CheckRadioButton(hDlg, IDC_RADIO_LOADLIBRARY, IDC_RADIO_LOADLIBRARY, IDC_RADIO_LOADLIBRARY);
        
        Shared.bLocalSaveInjection  = LOADLIBRARY_INJECTION_ID;
        Shared.pSaveInjection       = lParam;
        
        return TRUE;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(uiParam))
        {

        case ID_CANCEL:
        {
            if (HIWORD(uiParam) == BN_CLICKED)
            {
                EndDialog(hDlg, 0);
                return TRUE;
            }
        }

        case ID_SAVE:
        {
            if (HIWORD(uiParam) == BN_CLICKED)
            {
                *(Shared.pSaveInjection) = Shared.bLocalSaveInjection;

                EndDialog(hDlg, 0);
                return TRUE;
            }
        }

        }
    }

    
    case WM_CLOSE:
    {
        EndDialog(hDlg, 0);
        return TRUE;
    }

    }

    return FALSE;
}



typedef struct ModuleDialogInfo
{
    WndMainProcInfo*    pParentShared;
    DWORD               dwProcessId;
    HWND                hList;
    ULONGLONG           ullAddr;
}ModuleDialogInfo;
INT_PTR ModuleDialog(HWND hDlg, UINT uiMessage, WPARAM uiParam, LPARAM lParam)
{
    static ModuleDialogInfo Shared;

    switch (uiMessage)
    {
    
    case WM_INITDIALOG:
    {
        Shared.dwProcessId      = NULL;
        Shared.ullAddr          = NULL;
        Shared.pParentShared    = lParam;
        Shared.hList            = GetDlgItem(hDlg, IDC_MODULELIST);

        TCHAR ptszPid[32];
        ZeroMemory(ptszPid, _countof(ptszPid));
        for (int i = 0, j = 0; i < _tcslen(Shared.pParentShared->ptszSelectedProcess); i++)
        {
            if (Shared.pParentShared->ptszSelectedProcess[i] == __TEXT(']'))
            {
                ptszPid[j] = __TEXT('\0');
                break;
            }

            if (Shared.pParentShared->ptszSelectedProcess[i] >= __TEXT('0') && Shared.pParentShared->ptszSelectedProcess[i] <= __TEXT('9'))
            {
                ptszPid[j] = Shared.pParentShared->ptszSelectedProcess[i];
                j++;
            }
            
        }

        //_ttoi refusing to work in this case so I have to recreate it, no idea why...
        for (int i = 0, j = _tcslen(ptszPid) - 1; i < _tcslen(ptszPid); i++, j--)
        {
            Shared.dwProcessId += ((pow(10, j))) * (ptszPid[i] - 0x30);
        }

        UpdateModuleList(Shared.hList, Shared.dwProcessId);
        SetTimer(hDlg, SUBSECTION_MODULE_TIMER_ID, 5000, NULL);

        return TRUE;
    }

    case WM_TIMER:
    {
        int iOldSelection   = SendMessage(Shared.hList, LB_GETCURSEL, NULL, NULL);
        int iOldTop         = SendMessage(Shared.hList, LB_GETTOPINDEX, NULL, NULL);

        SendMessage(Shared.hList, WM_SETREDRAW, FALSE, 0);
        UpdateModuleList(Shared.hList, Shared.dwProcessId);
        if (iOldSelection != LB_ERR)
        {
            SendMessage(Shared.hList, LB_SETCURSEL, iOldSelection, NULL);
        }
        SendMessage(Shared.hList, LB_SETTOPINDEX, iOldTop, NULL);
        SendMessage(Shared.hList, WM_SETREDRAW, TRUE, 0);

        return TRUE;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(uiParam))
        {

        case IDC_MODULEUNLOAD:
        {
            DWORD dwUnloadSelection;
            if ((dwUnloadSelection = SendMessage(Shared.hList, LB_GETCURSEL, NULL, NULL)) == LB_ERR)
            {
                break;
            }

            DWORD dwSelectionLength;
            if ((dwSelectionLength = SendMessage(Shared.hList, LB_GETTEXTLEN, dwUnloadSelection, NULL)) == LB_ERR)
            {
                break;
            }

            TCHAR ptszAddr[17];
            PTSTR ptszModule = (PTSTR)malloc((dwSelectionLength + 1) * sizeof(TCHAR));
            if (!ptszModule)
            {
                break;
            }
            __try
            {
                if (SendMessage(Shared.hList, LB_GETTEXT, dwUnloadSelection, ptszModule) == LB_ERR)
                {
                    __leave;
                }

                for (int i = 0, j = 0; i < _tcslen(ptszModule); i++)
                {
                    if (ptszModule[i] == __TEXT(']'))
                    {
                        ptszAddr[j] = __TEXT('\0');
                        break;
                    }

                    if ((ptszModule[i] >= __TEXT('0') && ptszModule[i] <= __TEXT('9')) || (ptszModule[i] >= __TEXT('A') && ptszModule[i] <= __TEXT('F')))
                    {
                        ptszAddr[j] = ptszModule[i];
                        j++;
                    }
                }
            }
            __finally
            {
                if (ptszModule)
                {
                    free(ptszModule);
                }
            }

            //strtoll refusing to work in this case so I have to recreate it, no idea why...
            for (int i = 0, j = _tcslen(ptszAddr) - 1; i < _tcslen(ptszAddr); i++, j--)
            {
                if (ptszAddr[i] >= __TEXT('0') && ptszAddr[i] <= __TEXT('9'))
                {
                    Shared.ullAddr += ((pow(16, j))) * (ptszAddr[i] - 0x30);
                }
                else
                {
                    Shared.ullAddr += ((pow(16, j))) * (ptszAddr[i] - 55);
                }
            }

            UnloadModule(Shared.dwProcessId, Shared.ullAddr);
            Sleep(500);
            SendMessage(hDlg, WM_TIMER, SUBSECTION_MODULE_TIMER_ID, NULL);
            
            break;
        }

        }

        return TRUE;
    }

    case WM_CLOSE:
    {
        EndDialog(hDlg, 0);
        return TRUE;
    }

    }

    return FALSE;
}



inline void SetWndClass(WNDCLASS* WndClass, UINT uiStyle, WNDPROC pfnWndProc, int cbClsExtra, int cbWndExtra, HINSTANCE hInstance, HICON hIcon, HCURSOR hCursor, HBRUSH hBackground, PTSTR ptszMenuName, PTSTR ptszClassName)
{
    WndClass->style         = uiStyle;
    WndClass->lpfnWndProc   = pfnWndProc;
    WndClass->cbClsExtra    = cbClsExtra;
    WndClass->cbWndExtra    = cbWndExtra;
    WndClass->hInstance     = hInstance;
    WndClass->hIcon         = hIcon;
    WndClass->hCursor       = hCursor;
    WndClass->hbrBackground = hBackground;
    WndClass->lpszMenuName  = ptszMenuName;
    WndClass->lpszClassName = ptszClassName;
}



BOOL ValidateInjectionAttempt(PWndMainProcInfo pSettings)
{
    if (!pSettings)
    {
        return FALSE;
    }

    if ((pSettings->bInjectionFlag > HIGHEST_VALID_INJECTION_ID) || !(pSettings->ptszSelectedDll[0]) || !(pSettings->ptszSelectedDllPath[0]) || !(pSettings->ptszSelectedProcess[0]))
    {
        return FALSE;
    }

    return TRUE;
}