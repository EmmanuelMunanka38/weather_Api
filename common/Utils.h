#pragma once

#include <string>

namespace common
{

inline std::string trim(const std::string &str)
{
    auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos)
        return "";
    auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

inline std::string toLower(const std::string &str)
{
    std::string result = str;
    for (auto &c : result)
        c = std::tolower(static_cast<unsigned char>(c));
    return result;
}

inline bool isValidEmail(const std::string &email)
{
    auto atPos = email.find('@');
    if (atPos == std::string::npos || atPos == 0)
        return false;
    auto dotPos = email.rfind('.');
    return dotPos > atPos + 1 && dotPos < email.size() - 1;
}

} // namespace common
