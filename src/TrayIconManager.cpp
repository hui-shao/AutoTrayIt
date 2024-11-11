#include "main.h"
#include "TrayIconManager.h"
#include "WindowManager.h"

NOTIFYICONDATAW TrayIconManager::nidMain;
HMENU TrayIconManager::hTrayMenuMain;


void TrayIconManager::InitMainTrayIcon(HINSTANCE hInstance, HWND hwndMain)
{
    nidMain.cbSize = sizeof(NOTIFYICONDATAW);
    nidMain.hWnd = hwndMain;
    nidMain.uID = MAIN_TRAY_ICON_ID;
    nidMain.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nidMain.uCallbackMessage = WM_TRAYICON;
    nidMain.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcscpy_s(nidMain.szTip, L"AutoTrayIt");

    // 创建托盘菜单
    hTrayMenuMain = CreatePopupMenu();
    AppendMenuW(hTrayMenuMain, MF_STRING, ID_TRAY_EXIT, L"退出");

    Shell_NotifyIconW(NIM_ADD, &nidMain);
}

void TrayIconManager::AddTrayIcon(HWND hwndTarget, HICON hIcon, const std::wstring& title)
{
    NOTIFYICONDATAW hiddenNid = {};
    hiddenNid.cbSize = sizeof(NOTIFYICONDATAW);
    hiddenNid.hWnd = nidMain.hWnd; // 注意传入参数应使用主程序窗口句柄，以便接收托盘图标消息
    hiddenNid.uID = MAIN_TRAY_ICON_ID + static_cast<unsigned int>(WindowManager::hiddenWindows.size()) + 1;
    hiddenNid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    hiddenNid.uCallbackMessage = WM_TRAYICON;
    hiddenNid.hIcon = hIcon;
    wcscpy_s(hiddenNid.szTip, title.c_str());
    Shell_NotifyIconW(NIM_ADD, &hiddenNid);
    // 将隐藏的窗口信息保存到 hiddenWindows
    WindowManager::hiddenWindows.push_back({hwndTarget, hiddenNid, false});
}

void TrayIconManager::RemoveAllTrayIcons()
{
    for (auto& hiddenWindow : WindowManager::hiddenWindows) // 恢复所有被隐藏的窗口
    {
        ShowWindow(hiddenWindow.hwnd, SW_SHOW);
        Shell_NotifyIconW(NIM_DELETE, &hiddenWindow.nid);
    }
    WindowManager::hiddenWindows.clear();
    Shell_NotifyIconW(NIM_DELETE, &nidMain); // 删除主程序托盘图标
}

void TrayIconManager::HandleTrayIconMessage(WPARAM wParam, LPARAM lParam)
{
    if (lParam == WM_RBUTTONUP)
    {
        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(nidMain.hWnd);
        TrackPopupMenu(hTrayMenuMain, TPM_RIGHTBUTTON, pt.x, pt.y, 0, nidMain.hWnd, nullptr);
    }
    else if (lParam == WM_LBUTTONDBLCLK)
    {
        if (wParam == nidMain.uID)
        {
            ShowWindow(nidMain.hWnd, SW_RESTORE); // 双击主程序托盘图标时，显示窗口
        }
        else
        {
            for (auto& hiddenWindow : WindowManager::hiddenWindows)
            {
                // 双击被隐藏程序的托盘图标时，切换窗口可见性
                if (hiddenWindow.nid.uID == wParam)
                {
                    if (IsWindowVisible(hiddenWindow.hwnd))
                    {
                        ShowWindow(hiddenWindow.hwnd, SW_HIDE);
                        hiddenWindow.manuallyShown = false;
                    }
                    else
                    {
                        ShowWindow(hiddenWindow.hwnd, SW_SHOW);
                        hiddenWindow.manuallyShown = true;
                    }
                    break;
                }
            }
        }
    }
}
