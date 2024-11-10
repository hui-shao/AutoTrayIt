#include <windows.h>
#include <shellapi.h>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <iostream>
#include "toml++/toml.h"

constexpr auto TRAY_ICON_ID = 1;
constexpr auto WM_TRAYICON = WM_USER + 1;

NOTIFYICONDATA nid;
HWND hwnd;
HINSTANCE hInstance;

std::atomic<bool> running(true); // 标志变量，用于通知线程退出
std::vector<std::string> keywords; // 存储关键字

// 从 TOML 文件中读取关键字
void LoadKeywords(const std::string& filename)
{
    try
    {
        auto config = toml::parse_file(filename);
        for (const auto& keyword : *config["keywords"].as_array())
        {
            keywords.push_back(keyword.value<std::string>().value_or(""));
        }
    }
    catch (const toml::parse_error& err)
    {
        std::cerr << "Error parsing TOML file: " << err << std::endl;
    }
}

// 窗口过程函数，处理窗口消息
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_TRAYICON:
        // 当托盘图标被双击时，恢复窗口
        if (lParam == WM_LBUTTONDBLCLK)
        {
            ShowWindow(hwnd, SW_RESTORE);
            Shell_NotifyIcon(NIM_DELETE, &nid);
        }
        break;
    case WM_SIZE:
        // 当窗口最小化时，隐藏窗口并添加托盘图标
        if (wParam == SIZE_MINIMIZED)
        {
            ShowWindow(hwnd, SW_HIDE);
            Shell_NotifyIcon(NIM_ADD, &nid);
        }
        break;
    case WM_CLOSE:
        // 当窗口关闭时，销毁窗口
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        // 当窗口销毁时，删除托盘图标并退出消息循环
        running = false; // 设置标志，通知线程退出
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// 检查所有窗口标题中是否包含关键字，如果包含则隐藏窗口
void CheckWindowsForKeyword()
{
    while (running)
    {
        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
        {
            char title[256];
            GetWindowText(hwnd, title, sizeof(title));
            for (const auto& keyword : *reinterpret_cast<std::vector<std::string>*>(lParam))
            {
                if (strstr(title, keyword.c_str()))
                {
                    ShowWindow(hwnd, SW_HIDE);
                    break;
                }
            }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&keywords));
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "TrayItExampleClass";

    RegisterClass(&wc);

    // 创建主窗口
    hwnd = CreateWindowEx(0, wc.lpszClassName, "AutoTrayIt", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 300,
                          200, nullptr, nullptr, hInstance, nullptr);

    // 初始化托盘图标数据
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_ID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    strcpy_s(nid.szTip, "Tray It Example");

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // 从 TOML 文件中加载关键字
    LoadKeywords("config.toml");

    // 启动后台线程检查窗口标题中的关键字
    std::thread keywordChecker(CheckWindowsForKeyword);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    keywordChecker.join(); // 等待线程结束
    return 0;
}