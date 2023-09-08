#pragma once

#include <memory>

bool ci_str_equal(const char* a, const char* b, const unsigned n);
bool has_white_space(const char* str);

// Templates
template <typename T, unsigned N>
constexpr unsigned comptime_len(T (&)[N])
{
    return N;
}

template <unsigned N, typename... Ts>
std::unique_ptr<char[]> f_str(const char (&str)[N], unsigned headroom, Ts... args)
{
    auto buffer = std::make_unique<char[]>(N + headroom);
    sprintf(buffer.get(), str, args...);
    return buffer;
}
