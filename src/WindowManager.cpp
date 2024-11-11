﻿#include "main.h"
#include "WindowManager.h"
#include "TrayIconManager.h"
#include <thread>
#include <chrono>


std::vector<HiddenWindowInfo> WindowManager::hiddenWindows;

void WindowManager::CheckWindowsForKeyword(const std::stop_token& stopToken, const std::vector<std::wstring>& keywords)
{
    while (!stopToken.stop_requested())
    {
        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL
        {
            if (!IsWindowVisible(hwnd)) // 检查窗口是否可见，只处理可见窗口
                return TRUE;

            wchar_t title[256];
            GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t));

            if (wcslen(title) == 0) // 跳过没有标题的窗口
                return TRUE;

            for (const auto& keyword : *reinterpret_cast<std::vector<std::wstring>*>(lParam))
            {
                if (wcsstr(title, keyword.c_str())) // 如果标题中包含关键字
                {
                    // 检查窗口是否已经被手动显示，如果是，则跳过
                    auto it = std::ranges::find_if(hiddenWindows,
                                                   [hwnd](const HiddenWindowInfo& info) { return info.hwnd == hwnd; });
                    if (it != hiddenWindows.end() && it->manuallyShown)
                    {
                        continue;
                    }

                    ShowWindow(hwnd, SW_HIDE);

                    // 获取被隐藏程序的窗口图标
                    auto hIcon = reinterpret_cast<HICON>(SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0));
                    if (!hIcon)
                    {
                        hIcon = reinterpret_cast<HICON>(GetClassLongPtr(hwnd, GCLP_HICONSM));
                    }

                    // 创建被隐藏程序的托盘图标数据
                    // 注意用 Main 的 hwnd，以便接收托盘图标消息，回调 WindowProc
                    TrayIconManager::AddTrayIcon(hwnd, hIcon, title);

                    break;
                }
            }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&keywords));
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}