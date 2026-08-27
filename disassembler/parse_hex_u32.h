#ifndef PARSE_HEX_U32_H
#define PARSE_HEX_U32_H

#include <charconv>
#include <cstdint>
#include <string>
#include <system_error>

inline bool ParseHexU32(const std::string &s, uint32_t &out)
{
    if (s.empty()) {
        return false;
    }
    uint32_t value = 0;
    const char *first = s.data();
    const char *last = first + s.size();
    auto result = std::from_chars(first, last, value, 16);
    if (result.ec != std::errc() || result.ptr != last) {
        return false;
    }
    out = value;
    return true;
}

inline bool ParseDecU32(const std::string &s, uint32_t &out)
{
    if (s.empty()) {
        return false;
    }
    uint32_t value = 0;
    const char *first = s.data();
    const char *last = first + s.size();
    auto result = std::from_chars(first, last, value, 10);
    if (result.ec != std::errc() || result.ptr != last) {
        return false;
    }
    out = value;
    return true;
}

inline bool ParseDecI32(const std::string &s, int32_t &out)
{
    if (s.empty()) {
        return false;
    }
    int32_t value = 0;
    const char *first = s.data();
    const char *last = first + s.size();
    auto result = std::from_chars(first, last, value, 10);
    if (result.ec != std::errc() || result.ptr != last) {
        return false;
    }
    out = value;
    return true;
}

#endif
