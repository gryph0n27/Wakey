#pragma once

#include "HotKey.g.h"

namespace winrt::wakey::implementation
{
    struct HotKey : HotKeyT<HotKey>
    {
    private:
        ::winrt::Microsoft::UI::Xaml::Window m_window;

        static bool         IsKeyDown   (_In_ ::winrt::Windows::System::VirtualKey key);
        static UINT         GetModifiers(void);
        static std::wstring Format      (_In_ UINT uHotKeyMod, _In_ UINT uHotKey);

    public:
        HotKey(_In_::winrt::Microsoft::UI::Xaml::Window const& window);
        void Close(void);

        void Restore    (_In_ ::winrt::Microsoft::UI::Xaml::Controls::TextBox const& txt);
        bool ScanKeyDown(_In_ ::winrt::Windows::System::VirtualKey key, _In_ ::winrt::Microsoft::UI::Xaml::Controls::TextBox const& txt);
        void ScanKeyUp  (_In_ ::winrt::Windows::System::VirtualKey key, _In_ ::winrt::Microsoft::UI::Xaml::Controls::TextBox const& txt);
        bool Register   (void);
        void UnRegister (void);
    };
}

namespace winrt::wakey::factory_implementation
{
    struct HotKey : HotKeyT<HotKey, implementation::HotKey>
    {
    };
}
