#include "pch.h"
#include "utils.h"

bool ci_str_equal(const char* a, const char* b, const unsigned n)
{
    for (int i = 0; i < n; i++)
    {
        if (a[i] != b[i] && a[i] + 32 != b[i] && a[i] - 32 != b[i]) {
            return false;
        }
    }
    return true;
}

bool has_white_space(const char* str)
{
    while (*str)
    {
        if (isspace(*str))
        {
            return true;
        }
        str++;
    }
    return false;
}

