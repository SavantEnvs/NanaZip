// Generic harness: -DCREATE=CreateRomfs etc. Open -> properties -> Extract(all, test + extract).
#include "NanaZip.Codecs.h"
#include <cstdio>
#include <vector>
struct MemIn : IInStream {
    const uint8_t* d; size_t n; uint64_t pos = 0;
    MemIn(const uint8_t* d, size_t n) : d(d), n(n) {}
    HRESULT QueryInterface(REFIID, void** p) override { if (p) *p = nullptr; return E_NOINTERFACE; }
    ULONG AddRef() override { return 1; } ULONG Release() override { return 1; }
    HRESULT Read(LPVOID b, UINT32 s, PUINT32 ps) override {
        size_t r = pos < n ? n - pos : 0; if (r > s) r = s; if (r) memcpy(b, d + pos, r); pos += r; if (ps) *ps = (UINT32)r; return S_OK; }
    HRESULT Seek(INT64 off, UINT32 org, PUINT64 np) override {
        int64_t base = org == 0 ? 0 : org == 1 ? (int64_t)pos : (int64_t)n; int64_t t = base + off;
        if (org > 2 || t < 0) return HRESULT_FROM_WIN32(131); pos = (uint64_t)t; if (np) *np = pos; return S_OK; }
};
struct Sink : ISequentialOutStream {
    uint64_t total = 0;
    HRESULT QueryInterface(REFIID, void** p) override { if (p) *p = nullptr; return E_NOINTERFACE; }
    ULONG AddRef() override { return 1; } ULONG Release() override { return 1; }
    HRESULT Write(LPCVOID b, UINT32 s, PUINT32 ps) override {
        volatile uint8_t x = 0; for (UINT32 i = 0; i < s; ++i) x ^= ((const uint8_t*)b)[i];
        total += s; if (ps) *ps = s; return total > (64u << 20) ? E_ABORT : S_OK; }
};
struct Cb : IArchiveExtractCallback {
    Sink sink;
    HRESULT QueryInterface(REFIID, void** p) override { if (p) *p = nullptr; return E_NOINTERFACE; }
    ULONG AddRef() override { return 1; } ULONG Release() override { return 1; }
    HRESULT SetTotal(UINT64) override { return S_OK; }
    HRESULT SetCompleted(const PUINT64) override { return S_OK; }
    HRESULT GetStream(UINT32, ISequentialOutStream** s, INT32 mode) override { *s = mode == SevenZipExtractAskModeExtract ? &sink : nullptr; return S_OK; }
    HRESULT PrepareOperation(INT32) override { return S_OK; }
    HRESULT SetOperationResult(INT32) override { return S_OK; }
};
static void clear(PROPVARIANT& v) { if (v.vt == VT_BSTR) SysFreeString(v.bstrVal); memset(&v, 0, sizeof v); }
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    IInArchive* a = NanaZip::Codecs::Archive::CREATE();
    MemIn in(data, size);
    if (a->Open(&in, nullptr, nullptr) == S_OK) {
        UINT32 n = 0, np = 0, nap = 0; a->GetNumberOfItems(&n); a->GetNumberOfProperties(&np); a->GetNumberOfArchiveProperties(&nap);
        std::vector<PROPID> ids;
        for (UINT32 i = 0; i < np; ++i) { BSTR nm = nullptr; PROPID id = 0; VARTYPE t; if (a->GetPropertyInfo(i, &nm, &id, &t) == S_OK) ids.push_back(id); SysFreeString(nm); }
        for (UINT32 i = 0; i < nap; ++i) { BSTR nm = nullptr; PROPID id = 0; VARTYPE t; PROPVARIANT v{}; if (a->GetArchivePropertyInfo(i, &nm, &id, &t) == S_OK) a->GetArchiveProperty(id, &v); SysFreeString(nm); clear(v); }
        for (UINT32 i = 0; i < n && i < 4096; ++i) for (PROPID id : ids) { PROPVARIANT v{}; a->GetProperty(i, id, &v); clear(v); }
        Cb cb; a->Extract(nullptr, (UINT32)-1, FALSE, &cb);
        Cb cb2; a->Extract(nullptr, (UINT32)-1, TRUE, &cb2);
    }
    a->Close(); a->Release();
    return 0;
}
#if 0
int main(int argc, char** argv) { for (int i = 1; i < argc; ++i) { FILE* f = fopen(argv[i], "rb"); if (!f) continue; std::vector<uint8_t> b; uint8_t t[65536]; size_t r; while ((r = fread(t, 1, sizeof t, f)) > 0) b.insert(b.end(), t, t + r); fclose(f); LLVMFuzzerTestOneInput(b.data(), b.size()); printf("%s: done\n", argv[i]); } }
#endif
