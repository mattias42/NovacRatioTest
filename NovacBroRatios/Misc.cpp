#include <algorithm>
#include <iostream>
#include "Misc.h"

// helper methods defined elsewhere in NovacPPP
void ShowMessage(const std::string& msg)
{
    std::cout << msg << std::endl;
}

void ShowMessage(const char message[])
{
    std::cout << message << std::endl;
}

void ShowMessage(const novac::CString &message)
{
    std::cout << message << std::endl;
}


int Equals(const std::string &str1, const std::string &str2)
{
#ifdef _MSC_VER
    return (0 == _strnicmp(str1.c_str(), str2.c_str(), std::max(strlen(str1.c_str()), strlen(str2.c_str()))));
#else
    return (0 == strncasecmp(str1.c_str(), str2.c_str(), std::max(strlen(str1.c_str()), strlen(str2.c_str()))));
#endif
}


