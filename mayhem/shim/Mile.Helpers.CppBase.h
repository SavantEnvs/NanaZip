#pragma once
#include <Windows.h>
#include <string>
#include <string_view>
#include <vector>
#include <cstdarg>
namespace Mile {
std::string FormatString(char const* const Format, ...);
std::wstring ToWideString(std::uint32_t CodePage, std::string_view const& InputString);
std::string ToString(std::uint32_t CodePage, std::wstring_view const& InputString);
template <class Derived, typename... Interfaces>
class ComObject : public Interfaces... {
    ULONG m_ReferenceCount = 1;
public:
    virtual ~ComObject() = default;
    HRESULT QueryInterface(REFIID, void** ppv) override { if (ppv) *ppv = nullptr; return E_NOINTERFACE; }
    ULONG AddRef() override { return ++m_ReferenceCount; }
    ULONG Release() override { ULONG c = --m_ReferenceCount; if (!c) delete this; return c; }
};
}
namespace Mile {
std::int64_t ToInt64(std::string const& Source, std::uint8_t const& Radix = 10);
std::uint32_t ToUInt32(std::string const& Source, std::uint8_t const& Radix = 10);
std::uint64_t ToUInt64(std::string const& Source, std::uint8_t const& Radix = 10);
}
