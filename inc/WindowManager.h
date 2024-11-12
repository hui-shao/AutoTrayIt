#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <stop_token>

struct HiddenWindowInfo
{
    HWND hwnd;
    NOTIFYICONDATAW nid;
    bool manuallyShown;
};

class WindowManager
{
public:
    static void CheckWindowsForKeyword(const std::stop_token& stopToken,
                                       const std::vector<std::wstring>& keywords);
    static void ToggleWindowVisibilityByTrayIconId(UINT trayIconId);
    static void RestoreAllHiddenWindows();

    static std::vector<HiddenWindowInfo> hiddenWindows;
};

#endif //WINDOWMANAGER_H
