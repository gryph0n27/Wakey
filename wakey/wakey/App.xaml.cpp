#include "pch.h"
#include "App.xaml.h"

#include "MainWindow.xaml.h"

using namespace winrt::wakey;

#define APP_INSTANCE_GUID   L"{D5D941CA-6BB9-4E44-9876-CBB235A25410}"

///////////////////////////////////////////////////////////////////////////////

namespace winrt::wakey::implementation
{
    ///////////////////////////////////////////////////////////////////////////////

    App* App::s_pThis = nullptr;

    ///////////////////////////////////////////////////////////////////////////////

    App::App(VOID)
    {
        // Xaml objects should not call InitializeComponent during construction.
        // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent
     
        //Settings::Clear();

        s_pThis = this;
                
        int32_t iTheme = Settings::Get(
            Settings::SettingType::Theme, -1
        );
        WINRT_ASSERT(iTheme >= -1 && iTheme < 2);

        if (iTheme != -1)
        {
            ::winrt::Microsoft::UI::Xaml::Application::Current().RequestedTheme(
                (::winrt::Microsoft::UI::Xaml::ApplicationTheme)iTheme
            );
        }

#if defined _DEBUG 
        winrt::Microsoft::Windows::ApplicationModel::Resources::ResourceManager resMsg;
        resMsg.ResourceNotFound([](
            winrt::Microsoft::Windows::ApplicationModel::Resources::ResourceManager const&,
            winrt::Microsoft::Windows::ApplicationModel::Resources::ResourceNotFoundEventArgs const& args)
            {
                OutputDebugStringW(args.Name().c_str());
            });
#endif
#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
        UnhandledException([](
                winrt::Windows::Foundation::IInspectable const&,
                winrt::Microsoft::UI::Xaml::UnhandledExceptionEventArgs const& e
            )
            {
                if (IsDebuggerPresent())
                {
                    auto errorMessage = e.Message();
                    __debugbreak();
                }
            });
#endif
    }

    ///////////////////////////////////////////////////////////////////////////////

    winrt::fire_and_forget App::RedirectActivationFAF(
        _In_ winrt::Microsoft::Windows::AppLifecycle::AppInstance const& currentInstance,
        _In_ winrt::Microsoft::Windows::AppLifecycle::AppActivationArguments const& activationArgs
    )
    {
        if (currentInstance && activationArgs)
            co_await currentInstance.RedirectActivationToAsync(activationArgs);
    }

    ///////////////////////////////////////////////////////////////////////////////

    VOID App::OnLaunched(
        _In_ winrt::Microsoft::UI::Xaml::LaunchActivatedEventArgs const& args
    )
    {
        UNREFERENCED_PARAMETER(args);

        // Register our unique GUID for single instance:

        winrt::Microsoft::Windows::AppLifecycle::AppInstance currentInstance =
            winrt::Microsoft::Windows::AppLifecycle::AppInstance::FindOrRegisterForKey(
                APP_INSTANCE_GUID
            );

        winrt::Microsoft::Windows::AppLifecycle::AppActivationArguments activationArgs =
            currentInstance.GetActivatedEventArgs();

        // Not the first instance, exit:
        if (!currentInstance.IsCurrent())
        {
            RedirectActivationFAF(currentInstance, activationArgs);
            Program::Exit();

            return;
        }
        else
        {
            currentInstance.Activated(
                [&currentInstance](
                    ::winrt::Windows::Foundation::IInspectable const& sender,
                    ::winrt::Microsoft::Windows::AppLifecycle::AppActivationArguments const& args
                )
                {
                    s_pThis->OnActivated(sender, args);
                }
            );
        }

        // Enable dark mode for popup menu:
        Theme::AllowDarkModeForApp();

        BOOL bRestart = FALSE;
        if (activationArgs.Kind() == winrt::Microsoft::Windows::AppLifecycle::ExtendedActivationKind::Launch)
        {
            ::winrt::Windows::ApplicationModel::Activation::ILaunchActivatedEventArgs launchArgs =
                activationArgs.Data().try_as<::winrt::Windows::ApplicationModel::Activation::ILaunchActivatedEventArgs>();

            if (launchArgs)
            {
                ::winrt::hstring argString = launchArgs.Arguments();
                if (argString.size())
                {
                    LPWSTR pwszArgs = ::PathGetArgsW(argString.c_str());
                    if (!::StrCmpIW(pwszArgs, L"--theme"))
                        bRestart = TRUE;
                }
            }
        }
        
        // First run, show window:

        BOOL bActivateWnd = FALSE;
        bool bFirstRun = Settings::Get(Settings::SettingType::FirstRun, true);
        if (bFirstRun)
        {
            bActivateWnd = TRUE;
            Settings::Set(
                Settings::SettingType::FirstRun, false
            );
        }
       
        OSVERSIONINFOEX osvix = { 0 };
        if (Misc::GetWindowsVersion(&osvix))
        {
            // Starting with Windows 11 build 27686.1000 (Canary) and build
            // 26100.1876 (RP), Microsoft is beginning to roll out improve-
            // ments to Settings > System > Power & battery including the 
            // ability to set your Power Mode for both when your PC is 
            // plugged in when it’s on battery along with a few other UI
            // improvements to the page.

            if (osvix.dwMajorVersion < 10 ||
               (osvix.dwMajorVersion == 10 &&
                osvix.dwBuildNumber < 26100))
            {
                Settings::Set(
                    Settings::SettingType::AllowPowerOverlayUpdate, true
                );
            }
            else
            {
                if (Settings::Get(Settings::SettingType::PowerOverlay, false))
                    Settings::Set(Settings::SettingType::PowerOverlay, false);
            }
        }

        // TODO:
        {
            Settings::Set(
                Settings::SettingType::TimeInterval, 0
            );           
        }

        window = winrt::make<MainWindow>(bRestart);
        if (bActivateWnd)
            Wnd::Activate(window);
    }

    ///////////////////////////////////////////////////////////////////////////////

    VOID App::Exit(VOID)
    {
       
    }

    ///////////////////////////////////////////////////////////////////////////////

    VOID App::OnActivated(
        _In_ winrt::Windows::Foundation::IInspectable const& sender,
        _In_ winrt::Microsoft::Windows::AppLifecycle::AppActivationArguments const& args
    )
    {
        UNREFERENCED_PARAMETER(sender);
        if (args.Kind() == winrt::Microsoft::Windows::AppLifecycle::ExtendedActivationKind::Launch)
        {
            WINRT_ASSERT(FALSE);            
        }
        else if (args.Kind() == winrt::Microsoft::Windows::AppLifecycle::ExtendedActivationKind::ToastNotification)
        {
            WINRT_ASSERT(s_pThis);
            if (s_pThis)
            {
                HWND hWnd = Wnd::GetHwnd(s_pThis->window);
                Wnd::Activate(hWnd);
            }            
        }               
    }
}

///////////////////////////////////////////////////////////////////////////////

