#pragma once

#ifdef UNICODE
#define WIN_MAIN                    wWinMain
#else
#define WIN_MAIN                    WinMain
#endif

#ifndef MAIN_WNDCLASS_NAME
#define MAIN_WNDCLASS_NAME          __TEXT("Injector_Main")
#endif
#ifndef MAIN_WND_NAME
#define MAIN_WND_NAME               __TEXT("Injector")
#endif

#ifndef SUBSECTION_WNDCLASS_NAME
#define SUBSECTION_WNDCLASS_NAME    __TEXT("Injector_Subsection")
#endif

#ifndef SUBSECTION_PROC_ID
#define SUBSECTION_PROC_ID          (HMENU)1
#endif
#ifndef SUBSECTION_PROC_BUTTON_ID  
#define SUBSECTION_PROC_BUTTON_ID   (HMENU)3
#endif
#ifndef SUBSECTION_PROC_TIMER_ID    
#define SUBSECTION_PROC_TIMER_ID    5
#endif
#ifndef SUBSECTION_PROC_DISPLAY_ID 
#define SUBSECTION_PROC_DISPLAY_ID  (HMENU)6
#endif

#ifndef SUBSECTION_FILE_ID  
#define SUBSECTION_FILE_ID          (HMENU)2
#endif
#ifndef SUBSECTION_FILE_BUTTON_ID
#define SUBSECTION_FILE_BUTTON_ID   (HMENU)4
#endif
#ifndef SUBSECTION_FILE_DISPLAY_ID  
#define SUBSECTION_FILE_DISPLAY_ID  (HMENU)7
#endif

#ifndef SUBSECTION_MODULE_TIMER_ID
#define SUBSECTION_MODULE_TIMER_ID  8
#endif


//Custom messsages
#ifndef QM_PROCUPDATE 
#define QM_PROCUPDATE (WM_USER + 1)
#endif
#ifndef QM_FILEUPDATE
#define QM_FILEUPDATE (WM_USER + 2)
#endif