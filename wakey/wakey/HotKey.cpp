#include "pch.h"
#include "HotKey.h"
#if __has_include("HotKey.g.cpp")
#include "HotKey.g.cpp"
#endif

using namespace winrt;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

#define HOTKEY_ID   1

namespace winrt::wakey::implementation
{
    using ::winrt::Windows::System::VirtualKey;

    ///////////////////////////////////////////////////////////////////////////////

    HotKey::HotKey(_In_::winrt::Microsoft::UI::Xaml::Window const& window)
          : m_window(window)
    {
    }

    ///////////////////////////////////////////////////////////////////////////////

    void HotKey::Close(void)
    {
        UnRegister();
    }

    ///////////////////////////////////////////////////////////////////////////////

    bool HotKey::IsKeyDown(
        _In_ VirtualKey key
    )
    {
        winrt::Windows::UI::Core::CoreVirtualKeyStates keyState =
            winrt::Microsoft::UI::Input::InputKeyboardSource::GetKeyStateForCurrentThread(key);

        uint32_t keyDown =
            (uint32_t)winrt::Windows::UI::Core::CoreVirtualKeyStates::Down;

        return ((uint32_t)keyState & keyDown) == keyDown;
    }

    ///////////////////////////////////////////////////////////////////////////////

    UINT HotKey::GetModifiers(void)
    {
        UINT uKeyMod = 0;

        if (IsKeyDown(VirtualKey::Control))
            uKeyMod |= MOD_CONTROL;
        if (IsKeyDown(VirtualKey::Shift))
            uKeyMod |= MOD_SHIFT;
        if (IsKeyDown(VirtualKey::Menu))
            uKeyMod |= MOD_ALT;
        if (IsKeyDown(VirtualKey::LeftWindows) ||
            IsKeyDown(VirtualKey::RightWindows))
            uKeyMod |= MOD_WIN;

        return uKeyMod;
    }

    ///////////////////////////////////////////////////////////////////////////////

    std::wstring HotKey::Format(
        _In_ UINT uHotKeyMod,
        _In_ UINT uHotKey
    )
    {
        std::wstring wstrKey;

        if (uHotKeyMod & MOD_CONTROL)
            wstrKey += L"Ctrl + ";
        if (uHotKeyMod & MOD_SHIFT)
            wstrKey += L"Shift + ";
        if (uHotKeyMod & MOD_ALT)
            wstrKey += L"Alt + ";
        if (uHotKeyMod & MOD_WIN)
            wstrKey += L"Win + ";

        if (uHotKey >= (UINT)VirtualKey::A &&
            uHotKey <= (UINT)VirtualKey::Z)
            wstrKey += (wchar_t)uHotKey;

        return wstrKey;
    }

    ///////////////////////////////////////////////////////////////////////////////

    void HotKey::Restore(
        _In_ ::winrt::Microsoft::UI::Xaml::Controls::TextBox const& txt
    )
    {
        UINT uHotKey = Settings::Get(
            Settings::HotKey,
            (int32_t)VirtualKey::W
        );

        UINT uHotKeyMod = Settings::Get(
            Settings::HotKeyMod,
            (int32_t)(MOD_SHIFT | MOD_CONTROL)
        );

        if (uHotKeyMod > 0 && uHotKey > 0)
            txt.Text(Format(uHotKeyMod, uHotKey));
        else
            txt.Text(L"");
    }

    ///////////////////////////////////////////////////////////////////////////////

    bool HotKey::Register(void)
    {
        UINT uHotKey = Settings::Get(
            Settings::HotKey,
            (int32_t)VirtualKey::W
        );

        UINT uHotKeyMod = Settings::Get(
            Settings::HotKeyMod,
            (int32_t)(MOD_SHIFT | MOD_CONTROL)
        );

        UnRegister();

        if (uHotKey &&
            uHotKeyMod)
        {
            return ::RegisterHotKey(
                Wnd::GetHwnd(m_window), HOTKEY_ID, uHotKeyMod | MOD_NOREPEAT, uHotKey
            );
        }

        return false;
    }

    ///////////////////////////////////////////////////////////////////////////////

    void HotKey::UnRegister(void)
    {
        UnregisterHotKey(
            Wnd::GetHwnd(m_window), HOTKEY_ID
        );
    }

    ///////////////////////////////////////////////////////////////////////////////
    // Returns false if the new key combination could not be registered
    // (e.g. it is already taken by another application).

    bool HotKey::ScanKeyDown(
        _In_ VirtualKey key,
        _In_ ::winrt::Microsoft::UI::Xaml::Controls::TextBox const& txt
    )
    {
        if (key == VirtualKey::Tab)
            return true;

        if (txt.FocusState() == ::winrt::Microsoft::UI::Xaml::FocusState::Unfocused)
            return true;

        UINT uKeyMod = GetModifiers();

        // Backspace / Delete without modifiers clears the hotkey
        if (uKeyMod == 0 &&
            (key == VirtualKey::Back || key == VirtualKey::Delete))
        {
            Settings::Set(Settings::HotKey,    (int32_t)0);
            Settings::Set(Settings::HotKeyMod, (int32_t)0);
            UnRegister();
            txt.Text(L"");
            return true;
        }

        if (key >= VirtualKey::A &&
            key <= VirtualKey::Z &&
            uKeyMod != 0)
        {
            int32_t iOldKey = Settings::Get(Settings::HotKey,    (int32_t)VirtualKey::W);
            int32_t iOldMod = Settings::Get(Settings::HotKeyMod, (int32_t)(MOD_SHIFT | MOD_CONTROL));

            Settings::Set(Settings::HotKey,    (int32_t)key);
            Settings::Set(Settings::HotKeyMod, (int32_t)uKeyMod);

            if (Register())
            {
                txt.Text(Format(uKeyMod, (UINT)key));
                return true;
            }

            // Registration failed, revert to the previous combination
            Settings::Set(Settings::HotKey,    iOldKey);
            Settings::Set(Settings::HotKeyMod, iOldMod);
            Register();
            Restore(txt);
            return false;
        }

        // Show modifiers being held as a preview, e.g. "Ctrl + Shift + "
        if (uKeyMod != 0)
            txt.Text(Format(uKeyMod, 0));

        return true;
    }

    ///////////////////////////////////////////////////////////////////////////////
    // Once all modifiers are released without a letter, show the saved hotkey again

    void HotKey::ScanKeyUp(
        _In_ VirtualKey key,
        _In_ ::winrt::Microsoft::UI::Xaml::Controls::TextBox const& txt
    )
    {
        if (key == VirtualKey::Tab)
            return;

        if (txt.FocusState() == ::winrt::Microsoft::UI::Xaml::FocusState::Unfocused)
            return;

        if (GetModifiers() == 0)
            Restore(txt);
    }

    ///////////////////////////////////////////////////////////////////////////////
}
