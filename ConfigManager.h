#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#pragma once
#include <string>
#include <vector>

class ConfigManager
{
public:
    static void LoadKeywords(const std::string& filename, std::vector<std::wstring>& keywords);
};

#endif //CONFIGMANAGER_H
