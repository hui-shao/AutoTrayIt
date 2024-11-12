#ifndef MAIN_H
#define MAIN_H

#pragma once

#include <windows.h>

static constexpr auto MAIN_TRAY_ICON_ID = 1;
static constexpr auto WM_TRAYICON = WM_USER + 1; // 用于接收托盘图标消息的共用消息ID
static constexpr auto ID_TRAY_EXIT = 1001; // 定义退出菜单项的ID

#endif //MAIN_H
