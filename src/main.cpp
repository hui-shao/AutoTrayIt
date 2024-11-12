#include <windows.h>
#include <thread>
#include <clocale>

#include "main.h"
#include "TrayIconManager.h"
#include "WindowManager.h"
#include "ConfigManager.h"


// 项目版本号
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
const std::string VERSION = TOSTRING(PROJECT_VERSION);


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_TRAYICON:
        TrayIconManager::HandleTrayIconMessage(wParam, lParam);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_EXIT) // 退出菜单项
        {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        }
        break;
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            ShowWindow(hwnd, SW_HIDE);
            Shell_NotifyIconW(NIM_ADD, &TrayIconManager::nidMain);
        }
        break;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        TrayIconManager::RemoveAllTrayIcons(); // 删除所有托盘图标
        WindowManager::RestoreAllHiddenWindows(); // 恢复所有被隐藏的窗口
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam); // 注意使用W版本，否则窗口标题显示会有问题
    }
    return 0;
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    std::setlocale(LC_ALL, ".UTF-8");

    // 读取配置，加载关键字
    std::vector<std::wstring> keywords;
    ConfigManager::LoadKeywords("config.toml", keywords);

    // 创建主窗口
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"AutoTrayItMainClass";
    RegisterClassW(&wc);

    std::wstring windowTitle = L"AutoTrayIt v" + std::wstring(VERSION.begin(), VERSION.end());
    HWND hwndMain = CreateWindowExW(0, wc.lpszClassName, windowTitle.c_str(), WS_OVERLAPPEDWINDOW,
                                    CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
                                    nullptr, nullptr, hInstance, nullptr);

    // 初始化主程序托盘图标
    TrayIconManager::InitMainTrayIcon(hInstance, hwndMain);
    ShowWindow(hwndMain, SW_HIDE);

    // 启动关键字检查线程, jthread 会有 stop_token 参数
    std::jthread keywordChecker(WindowManager::CheckWindowsForKeyword, std::ref(keywords));

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 通知关键字检查线程退出
    keywordChecker.request_stop();

    return 0;
}
