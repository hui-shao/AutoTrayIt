#include <windows.h>
#include <shellapi.h>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <iostream>
#include <locale>
#include "toml++/toml.h"

constexpr auto TRAY_ICON_ID = 1;
constexpr auto WM_TRAYICON = WM_USER + 1;
constexpr auto ID_TRAY_EXIT = 1001; // 定义退出菜单项的ID

NOTIFYICONDATAW nid;
HWND hwnd;
HINSTANCE hInstance;
HMENU hTrayMenu; // 托盘菜单句柄

std::atomic running(true); // 标志变量，用于通知线程退出
std::vector<std::wstring> keywords; // 存储关键字

struct HiddenWindowInfo
{
    HWND hwnd;
    NOTIFYICONDATAW nid;
};

std::vector<HiddenWindowInfo> hiddenWindows; // 存储已经被隐藏的窗口信息

std::string WideStringToUtf8(const std::wstring& wstr)
{
    if (wstr.empty()) return {};
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), nullptr, 0, nullptr,
                                          nullptr);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), &strTo[0], size_needed, nullptr, nullptr);
    return strTo;
}

std::wstring Utf8ToWideString(const std::string& str)
{
    if (str.empty()) return {};
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), nullptr, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &wstrTo[0], size_needed);
    return wstrTo;
}

// 显示错误弹窗
void ShowErrorMessage(const std::wstring& message)
{
    MessageBoxW(nullptr, message.c_str(), L"Error", MB_ICONERROR | MB_OK);
}

// 从 TOML 文件中读取关键字
void LoadKeywords(const std::string& filename)
{
    try
    {
        std::ifstream ifs(filename);
        if (!ifs.is_open())
        {
            ShowErrorMessage(L"Error: File not found - " + Utf8ToWideString(filename));
            ExitProcess(EXIT_FAILURE);
        }

        std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

        // Parse the content as a string view
        auto config = toml::parse(std::string_view(content));
        for (const auto& keyword : *config["keywords"].as_array())
        {
            keywords.push_back(Utf8ToWideString(keyword.value<std::string>().value_or("")));
        }
    }
    catch (const toml::parse_error& err)
    {
        ShowErrorMessage(L"Error parsing TOML file: " + Utf8ToWideString(std::string(err.description())));
        ExitProcess(EXIT_FAILURE);
    }
    catch (const std::exception& e)
    {
        ShowErrorMessage(L"Error: " + Utf8ToWideString(e.what()));
        ExitProcess(EXIT_FAILURE);
    }
}

// 窗口过程函数，处理窗口消息
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP)
        {
            // 显示托盘菜单
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hTrayMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, nullptr);
        }
        else if (lParam == WM_LBUTTONDBLCLK)
        {
            // 双击托盘图标时，显示窗口
            ShowWindow(hwnd, SW_RESTORE);
        }
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_TRAY_EXIT)
        {
            // 处理退出菜单项
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        }
        break;
    case WM_SIZE:
        // 当窗口最小化时，隐藏窗口并添加托盘图标
        if (wParam == SIZE_MINIMIZED)
        {
            ShowWindow(hwnd, SW_HIDE);
            Shell_NotifyIconW(NIM_ADD, &nid);
        }
        break;
    case WM_CLOSE:
        // 当窗口关闭时，销毁窗口
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        // 当窗口销毁时
        for (auto& hiddenWindow : hiddenWindows) // 恢复所有被隐藏的窗口
        {
            ShowWindow(hiddenWindow.hwnd, SW_SHOW);
            Shell_NotifyIconW(NIM_DELETE, &hiddenWindow.nid); // 删除被隐藏程序的托盘图标
        }
        hiddenWindows.clear(); // 清空记录
        running = false; // 设置标志，通知后台线程退出
        Shell_NotifyIconW(NIM_DELETE, &nid); // 删除主程序托盘图标
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
            if (!IsWindowVisible(hwnd)) // 检查窗口是否可见
                return TRUE;

            wchar_t title[256];
            GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));
            if (wcslen(title) == 0)
                return TRUE;

            for (const auto& keyword : *reinterpret_cast<std::vector<std::wstring>*>(lParam))
            {
                if (wcsstr(title, keyword.c_str()))
                {
                    ShowWindow(hwnd, SW_HIDE);

                    // 获取被隐藏程序的窗口图标
                    auto hIcon = reinterpret_cast<HICON>(SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0));
                    if (!hIcon)
                    {
                        hIcon = reinterpret_cast<HICON>(GetClassLongPtr(hwnd, GCLP_HICONSM));
                    }

                    // 创建被隐藏程序的托盘图标数据
                    NOTIFYICONDATAW hiddenNid = {};
                    hiddenNid.cbSize = sizeof(NOTIFYICONDATAW);
                    hiddenNid.hWnd = hwnd;
                    hiddenNid.uID = TRAY_ICON_ID + static_cast<unsigned int>(hiddenWindows.size()) + 1; // 确保每个托盘图标的ID唯一
                    hiddenNid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
                    hiddenNid.uCallbackMessage = WM_TRAYICON;
                    hiddenNid.hIcon = hIcon;
                    wcscpy_s(hiddenNid.szTip, title);
                    Shell_NotifyIconW(NIM_ADD, &hiddenNid);
                    hiddenWindows.push_back({hwnd, hiddenNid}); // 记录被隐藏的窗口信息
                    break;
                }
            }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&keywords));
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // 设置全局区域设置为 UTF-8
    std::setlocale(LC_ALL, ".UTF-8");

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TrayItExampleClass";

    RegisterClassW(&wc);

    // 创建主窗口
    hwnd = CreateWindowExW(0, wc.lpszClassName, L"AutoTrayIt", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
                           nullptr, nullptr, hInstance, nullptr);

    // 初始化托盘图标数据
    nid.cbSize = sizeof(NOTIFYICONDATAW);
    nid.hWnd = hwnd;
    nid.uID = TRAY_ICON_ID;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcscpy_s(nid.szTip, L"AutoTrayIt");

    // 从 TOML 文件中加载关键字
    LoadKeywords("config.toml");

    // 隐藏主窗口
    ShowWindow(hwnd, SW_HIDE);
    Shell_NotifyIconW(NIM_ADD, &nid);

    // 创建托盘菜单
    hTrayMenu = CreatePopupMenu();
    AppendMenuW(hTrayMenu, MF_STRING, ID_TRAY_EXIT, L"退出");

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
