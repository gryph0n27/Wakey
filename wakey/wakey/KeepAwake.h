#pragma once

#include "KeepAwake.g.h"

namespace winrt::wakey::implementation
{
    struct KeepAwake : KeepAwakeT<KeepAwake>
    {
        HANDLE      m_hThread;
        HANDLE      m_hReload;        
        HWND        m_hWnd;
        BOOL        m_bFirst;

        KeepAwake(_In_ winrt::Microsoft::UI::Xaml::Window window);
        
        VOID    Start       (_In_ BOOL bFirst);
        VOID    Reload      (VOID);
        VOID    Stop        (VOID);
        VOID    Close       (VOID);
        VOID    StatusUpdate(VOID);

        static VOID  CALLBACK APCProc(_In_ ULONG_PTR Parameter);
        static DWORD CALLBACK ThreadProc(_In_ LPVOID pParam);        
    };
}

namespace winrt::wakey::factory_implementation
{
    struct KeepAwake : KeepAwakeT<KeepAwake, implementation::KeepAwake>
    {
    };
}
