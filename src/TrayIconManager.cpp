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

NOTIFYICONDATAW TrayIconManager::AddTrayIcon(const HICON& hIcon, const std::wstring& title)
{
    NOTIFYICONDATAW hiddenNid = {};
    hiddenNid.cbSize = sizeof(NOTIFYICONDATAW);
    hiddenNid.hWnd = nidMain.hWnd; // 注意：这里要使用主程序窗口句柄，以便在主窗口内注册的 WindowProc 能处理这些托盘图标消息
    hiddenNid.uID = MAIN_TRAY_ICON_ID + static_cast<unsigned int>(WindowManager::hiddenWindows.size()) + 1;
    hiddenNid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    hiddenNid.uCallbackMessage = WM_TRAYICON;
    hiddenNid.hIcon = hIcon;
    wcscpy_s(hiddenNid.szTip, title.c_str());
    Shell_NotifyIconW(NIM_ADD, &hiddenNid);

    return hiddenNid;
}

void TrayIconManager::RemoveTrayIcon(NOTIFYICONDATAW& nid)
{
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

// 删除所有托盘图标 包括主程序
void TrayIconManager::RemoveAllTrayIcons()
{
    for (auto& hiddenWindow : WindowManager::hiddenWindows) // 删除所有被隐藏的窗口的图标
    {
        Shell_NotifyIconW(NIM_DELETE, &hiddenWindow.nid);
    }
    Shell_NotifyIconW(NIM_DELETE, &nidMain); // 删除主程序托盘图标
}

void TrayIconManager::HandleTrayIconMessage(WPARAM wParam, LPARAM lParam)
{
    if (lParam == WM_RBUTTONUP)
    {
        if (wParam == nidMain.uID) // 仅主程序显示托盘图标的上下文菜单
        {
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(nidMain.hWnd);
            TrackPopupMenu(hTrayMenuMain, TPM_RIGHTBUTTON, pt.x, pt.y, 0, nidMain.hWnd, nullptr);
        }
    }
    else if (lParam == WM_LBUTTONDBLCLK)
    {
        if (wParam == nidMain.uID) ShowWindow(nidMain.hWnd, SW_RESTORE); // 双击主程序托盘图标时，显示窗口
        else WindowManager::ToggleWindowVisibilityByTrayIconId(static_cast<UINT>(wParam)); // 双击被隐藏程序的托盘图标时，切换窗口显示状态
    }
}
