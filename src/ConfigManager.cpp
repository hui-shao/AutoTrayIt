// ConfigManager.cpp
#include "ConfigManager.h"
#include "toml++/toml.h"
#include <fstream>
#include <locale>
#include <windows.h>


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

// 从 TOML 文件中读取关键字
void ConfigManager::LoadKeywords(const std::string& filename, std::vector<std::wstring>& keywords)
{
    try
    {
        std::ifstream ifs(filename);
        if (!ifs.is_open())
        {
            MessageBoxW(nullptr, L"Error:\ntoml.config not found.", L"Error - File Not Found", MB_ICONERROR | MB_OK);
            ExitProcess(EXIT_FAILURE);
        }

        std::string content((std::istreambuf_iterator(ifs)), std::istreambuf_iterator<char>());

        // Parse the content as a string view
        auto config = toml::parse(std::string_view(content));
        for (const auto& keyword : *config["keywords"].as_array())
        {
            keywords.push_back(Utf8ToWideString(keyword.value<std::string>().value_or("")));
        }
    }
    catch (const toml::parse_error& err)
    {
        MessageBoxW(nullptr, (L"Error parsing TOML file" + Utf8ToWideString(std::string(err.description()))).c_str(),
                    L"Error", MB_ICONERROR | MB_OK);
        ExitProcess(EXIT_FAILURE);
    }
    catch (const std::exception& e)
    {
        std::wstring errorMessage = L"Error:\n" + Utf8ToWideString(e.what());
        MessageBoxW(nullptr, errorMessage.c_str(), L"Error", MB_ICONERROR | MB_OK);
        ExitProcess(EXIT_FAILURE);
    }
}
