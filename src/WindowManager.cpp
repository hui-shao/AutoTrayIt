#include "main.h"
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
                    const auto hiddenNid = TrayIconManager::AddTrayIcon(hIcon, title);
                    hiddenWindows.push_back({hwnd, hiddenNid, false});

                    break;
                }
            }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&keywords));

        // 检查，如果被隐藏的窗口已关闭，则删除托盘图标，并从 hiddenWindows 移除
        for (auto it = hiddenWindows.begin(); it != hiddenWindows.end();)
        {
            if (!IsWindow(it->hwnd)) // 如果窗口已经被关闭
            {
                TrayIconManager::RemoveTrayIcon(it->nid);
                it = hiddenWindows.erase(it);
            }
            else
            {
                ++it;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

// 根据托盘图标ID， 切换被隐藏程序的窗口的可见性，并更新 manuallyShown 标志
void WindowManager::ToggleWindowVisibilityByTrayIconId(const UINT trayIconId)
{
    for (auto& [hwnd, nid, manuallyShown] : hiddenWindows)
    {
        if (nid.uID == trayIconId)
        {
            bool isVisible = IsWindowVisible(hwnd);
            ShowWindow(hwnd, isVisible ? SW_HIDE : SW_SHOW);
            manuallyShown = !isVisible;
            break;
        }
    }
}

// 恢复所有被隐藏的窗口，注意会清空 hiddenWindows 容器
void WindowManager::RestoreAllHiddenWindows()
{
    for (auto& hiddenWindow : hiddenWindows)
    {
        ShowWindow(hiddenWindow.hwnd, SW_SHOW);
        Shell_NotifyIconW(NIM_DELETE, &hiddenWindow.nid);
    }
    hiddenWindows.clear();
}
