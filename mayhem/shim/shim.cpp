#include <Mile.Helpers.CppBase.h>
#include <cstdio>
std::string Mile::FormatString(char const* const Format, ...) {
    va_list a, b; va_start(a, Format); va_copy(b, a);
    int n = vsnprintf(nullptr, 0, Format, a); va_end(a);
    std::string s(n > 0 ? n : 0, '\0'); if (n > 0) vsnprintf(s.data(), n + 1, Format, b); va_end(b); return s;
}
std::wstring Mile::ToWideString(std::uint32_t, std::string_view const& In) {
    std::wstring o; for (unsigned char c : In) o.push_back((wchar_t)c); return o; // byte-widening stand-in for MultiByteToWideChar
}
std::string Mile::ToString(std::uint32_t, std::wstring_view const& In) { std::string o; for (wchar_t c : In) o.push_back((char)c); return o; }
std::int64_t Mile::ToInt64(std::string const& s, std::uint8_t const& r) { return std::strtoll(s.c_str(), nullptr, r); }
std::uint32_t Mile::ToUInt32(std::string const& s, std::uint8_t const& r) { return (std::uint32_t)std::strtoul(s.c_str(), nullptr, r); }
std::uint64_t Mile::ToUInt64(std::string const& s, std::uint8_t const& r) { return std::strtoull(s.c_str(), nullptr, r); }
