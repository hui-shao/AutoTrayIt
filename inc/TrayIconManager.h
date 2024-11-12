#ifndef TRAYICONMANAGER_H
#define TRAYICONMANAGER_H

#pragma once
#include <windows.h>
#include <string>

class TrayIconManager
{
public:
    static void InitMainTrayIcon(HINSTANCE hInstance, HWND hwndMain);
    static NOTIFYICONDATAW AddTrayIcon(const HICON& hIcon, const std::wstring& title);
    static void RemoveTrayIcon(NOTIFYICONDATAW& nid);
    static void RemoveAllTrayIcons();
    static void HandleTrayIconMessage(WPARAM wParam, LPARAM lParam);

    static NOTIFYICONDATAW nidMain;

private:
    static HMENU hTrayMenuMain;
};

#endif //TRAYICONMANAGER_H
