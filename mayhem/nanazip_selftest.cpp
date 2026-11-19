// Functional self-test for NanaZip's archive handlers: opens an archive with the named handler,
// prints every item's path, directory flag and size, then extracts all items and prints the size
// and FNV-1a hash of each item's extracted bytes. mayhem/test.sh diffs this against golden output.
#include "NanaZip.Codecs.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
using namespace NanaZip::Codecs::Archive;
struct MemIn : IInStream {
    std::vector<uint8_t> d; uint64_t pos = 0;
    HRESULT QueryInterface(REFIID, void** p) override { if (p) *p = nullptr; return E_NOINTERFACE; }
    ULONG AddRef() override { return 1; } ULONG Release() override { return 1; }
    HRESULT Read(LPVOID b, UINT32 s, PUINT32 ps) override {
        size_t r = pos < d.size() ? d.size() - pos : 0; if (r > s) r = s; if (r) memcpy(b, d.data() + pos, r); pos += r; if (ps) *ps = (UINT32)r; return S_OK; }
    HRESULT Seek(INT64 off, UINT32 org, PUINT64 np) override {
        int64_t base = org == 0 ? 0 : org == 1 ? (int64_t)pos : (int64_t)d.size(); int64_t t = base + off;
        if (org > 2 || t < 0) return HRESULT_FROM_WIN32(131); pos = (uint64_t)t; if (np) *np = pos; return S_OK; }
};
struct Sink : ISequentialOutStream {
    uint64_t n = 0, h = 1469598103934665603ULL;
    HRESULT QueryInterface(REFIID, void** p) override { if (p) *p = nullptr; return E_NOINTERFACE; }
    ULONG AddRef() override { return 1; } ULONG Release() override { return 1; }
    HRESULT Write(LPCVOID b, UINT32 s, PUINT32 ps) override {
        for (UINT32 i = 0; i < s; ++i) { h ^= ((const uint8_t*)b)[i]; h *= 1099511628211ULL; } n += s; if (ps) *ps = s; return S_OK; }
};
struct Cb : IArchiveExtractCallback {
    std::map<UINT32, Sink> sinks; std::map<UINT32, INT32> results; UINT32 cur = 0;
    HRESULT QueryInterface(REFIID, void** p) override { if (p) *p = nullptr; return E_NOINTERFACE; }
    ULONG AddRef() override { return 1; } ULONG Release() override { return 1; }
    HRESULT SetTotal(UINT64) override { return S_OK; }
    HRESULT SetCompleted(const PUINT64) override { return S_OK; }
    HRESULT GetStream(UINT32 i, ISequentialOutStream** s, INT32 mode) override { cur = i; *s = mode == SevenZipExtractAskModeExtract ? &sinks[i] : nullptr; return S_OK; }
    HRESULT PrepareOperation(INT32) override { return S_OK; }
    HRESULT SetOperationResult(INT32 r) override { results[cur] = r; return S_OK; }
};
static std::string narrow(BSTR b) { std::string o; if (b) for (wchar_t* p = b; *p; ++p) o.push_back(*p < 128 ? (char)*p : '?'); return o; }
int main(int argc, char** argv) {
    if (argc != 3) { fprintf(stderr, "usage: %s <Handler> <archive>\n", argv[0]); return 2; }
    std::string k = argv[1]; IInArchive* a = nullptr;
    if (k == "Romfs") a = CreateRomfs(); else if (k == "Ufs") a = CreateUfs();
    else if (k == "DotNetSingleFile") a = CreateDotNetSingleFile(); else if (k == "ElectronAsar") a = CreateElectronAsar();
    else if (k == "Zealfs") a = CreateZealfs(); else if (k == "WebAssembly") a = CreateWebAssembly();
    else if (k == "Littlefs") a = CreateLittlefs();
    if (!a) { fprintf(stderr, "unknown handler %s\n", argv[1]); return 2; }
    MemIn in; FILE* f = fopen(argv[2], "rb"); if (!f) { perror(argv[2]); return 2; }
    uint8_t t[65536]; size_t r; while ((r = fread(t, 1, sizeof t, f)) > 0) in.d.insert(in.d.end(), t, t + r); fclose(f);
    HRESULT hr = a->Open(&in, nullptr, nullptr);
    printf("open %s: 0x%08x\n", k.c_str(), (unsigned)hr);
    if (hr != S_OK) { a->Release(); return 1; }
    UINT32 n = 0; a->GetNumberOfItems(&n); printf("items %u\n", n);
    for (UINT32 i = 0; i < n; ++i) {
        PROPVARIANT p{}, d{}, s{};
        a->GetProperty(i, SevenZipArchivePath, &p); a->GetProperty(i, SevenZipArchiveIsDirectory, &d); a->GetProperty(i, SevenZipArchiveSize, &s);
        printf("item %u path=%s dir=%d size=%llu\n", i, p.vt == VT_BSTR ? narrow(p.bstrVal).c_str() : "-",
               d.vt == VT_BOOL ? (d.boolVal != VARIANT_FALSE) : -1, s.vt == VT_UI8 ? (unsigned long long)s.uhVal.QuadPart : 0ULL);
        if (p.vt == VT_BSTR) SysFreeString(p.bstrVal);
    }
    Cb cb; hr = a->Extract(nullptr, (UINT32)-1, FALSE, &cb); printf("extract: 0x%08x\n", (unsigned)hr);
    for (auto& [i, sk] : cb.sinks) printf("data %u result=%d bytes=%llu fnv=%016llx\n", i, cb.results.count(i) ? cb.results[i] : -1, (unsigned long long)sk.n, (unsigned long long)sk.h);
    a->Close(); a->Release();
    return 0;
}
