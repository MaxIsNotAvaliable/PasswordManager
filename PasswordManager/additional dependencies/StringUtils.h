#pragma once
#include <string>
#include <WinNls.h>
#include <vector>

namespace convert
{
    inline std::string WideToANSI(const std::wstring& wstr)
    {
        int count = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
        std::string str(count, 0);
        WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
        return str;
    }

    inline std::wstring AnsiToWide(const std::string& str)
    {
        int count = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), NULL, 0);
        std::wstring wstr(count, 0);
        MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), &wstr[0], count);
        return wstr;
    }

    inline std::string WideToUtf8(const std::wstring& wstr)
    {
        int count = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
        std::string str(count, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], count, NULL, NULL);
        return str;
    }

    inline std::wstring Utf8ToWide(const std::string& str)
    {
        int count = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
        std::wstring wstr(count, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &wstr[0], count);
        return wstr;
    }

    inline std::string AnsiToUtf8(const std::string& str)
    {
        return WideToUtf8(AnsiToWide(str));
    }

    inline std::string Utf8ToAnsi(const std::string& str)
    {
        return WideToANSI(Utf8ToWide(str));
    }

    template<typename T>
    inline std::string DataToHex(T value)
    {
        size_t size = sizeof(T);
        std::string hex_str;

        for (size_t i = 0; i < size; i++)
        {
            char _buf[0x2 + 0x1];
            std::uint8_t uByte = reinterpret_cast<std::uint8_t*>(&value)[i];
            sprintf_s(_buf, "%.2X", uByte);
            hex_str.insert(0, _buf);
        }
        return hex_str;
    }

    inline unsigned int HexToData(const char* hex)
    {
        return std::stoul(hex, nullptr, 0x10);
    }
}