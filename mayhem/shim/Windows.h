#pragma once
// Minimal Linux shim of the Win32 types NanaZip.Codecs archive handlers use.
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cwchar>
#include <cstdlib>
typedef int32_t HRESULT; typedef int32_t LONG; typedef uint32_t ULONG; typedef uint32_t DWORD;
typedef int BOOL; typedef uint8_t BYTE, *PBYTE; typedef uint16_t WORD, USHORT, UINT16; typedef int16_t SHORT, INT16;
typedef uint32_t UINT32, UINT, *PUINT32; typedef int32_t INT32, INT; typedef uint64_t UINT64, *PUINT64, ULONGLONG; typedef int64_t INT64, *PINT64, LONGLONG;
typedef uint8_t UINT8; typedef int8_t INT8;
typedef size_t SIZE_T, *PSIZE_T; typedef void *PVOID, *LPVOID; typedef const void* LPCVOID;
typedef wchar_t WCHAR, OLECHAR; typedef WCHAR *BSTR, *LPWSTR; typedef const WCHAR *LPCWSTR, *LPCOLESTR; typedef const char* LPCSTR;
typedef uint16_t VARTYPE; typedef short VARIANT_BOOL; typedef ULONG PROPID; typedef char CHAR; typedef unsigned char UCHAR; typedef UCHAR BOOLEAN;
#define VARIANT_TRUE ((VARIANT_BOOL)-1)
#define VARIANT_FALSE ((VARIANT_BOOL)0)
#define TRUE 1
#define FALSE 0
#define MAX_PATH 260
#define CP_UTF8 65001
#define CP_ACP 0
#define WINAPI
#define STDMETHODCALLTYPE
#define EXTERN_C extern "C"
#define _In_
#define _In_opt_
#define _Out_
#define _Out_opt_
#define _Inout_
#define _Inout_opt_
#define _Out_writes_bytes_(x)
#define _In_reads_bytes_(x)
#define _Out_writes_bytes_to_(a,b)
#define UNREFERENCED_PARAMETER(x) (void)(x)
struct GUID { uint32_t Data1; uint16_t Data2, Data3; uint8_t Data4[8]; };
typedef GUID IID, CLSID; typedef const GUID& REFIID; typedef const GUID& REFGUID; typedef const GUID& REFCLSID;
inline bool operator==(const GUID& a, const GUID& b){ return !memcmp(&a,&b,sizeof(GUID)); }
#define MIDL_INTERFACE(x) struct
#define S_OK ((HRESULT)0)
#define S_FALSE ((HRESULT)1)
#define E_NOTIMPL ((HRESULT)0x80004001)
#define E_NOINTERFACE ((HRESULT)0x80004002)
#define E_POINTER ((HRESULT)0x80004003)
#define E_ABORT ((HRESULT)0x80004004)
#define E_FAIL ((HRESULT)0x80004005)
#define E_UNEXPECTED ((HRESULT)0x8000FFFF)
#define E_OUTOFMEMORY ((HRESULT)0x8007000E)
#define E_INVALIDARG ((HRESULT)0x80070057)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define STREAM_SEEK_SET 0
#define STREAM_SEEK_CUR 1
#define STREAM_SEEK_END 2
struct FILETIME { DWORD dwLowDateTime, dwHighDateTime; };
union LARGE_INTEGER { struct { DWORD LowPart; LONG HighPart; }; LONGLONG QuadPart; };
union ULARGE_INTEGER { struct { DWORD LowPart; DWORD HighPart; }; ULONGLONG QuadPart; };
enum { VT_EMPTY=0, VT_I2=2, VT_I4=3, VT_BSTR=8, VT_BOOL=11, VT_I1=16, VT_UI1=17, VT_UI2=18, VT_UI4=19, VT_I8=20, VT_UI8=21, VT_FILETIME=64 };
struct PROPVARIANT { VARTYPE vt; WORD r1, r2, r3; union { CHAR cVal; UCHAR bVal; SHORT iVal; USHORT uiVal; LONG lVal; ULONG ulVal; LARGE_INTEGER hVal; ULARGE_INTEGER uhVal; VARIANT_BOOL boolVal; FILETIME filetime; BSTR bstrVal; }; };
typedef PROPVARIANT *LPPROPVARIANT; typedef const PROPVARIANT* REFPROPVARIANT;
struct IUnknown { virtual HRESULT QueryInterface(REFIID riid, void** ppv) = 0; virtual ULONG AddRef() = 0; virtual ULONG Release() = 0; };
inline BSTR SysAllocStringLen(const OLECHAR* s, UINT n){ UINT32* p=(UINT32*)malloc(4+(n+1)*sizeof(OLECHAR)); if(!p) return nullptr; *p=n*sizeof(OLECHAR); BSTR b=(BSTR)(p+1); if(s) memcpy(b,s,n*sizeof(OLECHAR)); b[n]=0; return b; }
inline BSTR SysAllocString(const OLECHAR* s){ return s ? SysAllocStringLen(s,(UINT)wcslen(s)) : nullptr; }
inline void SysFreeString(BSTR b){ if(b) free(((UINT32*)b)-1); }
typedef const BYTE* LPCBYTE; typedef PROPVARIANT* PPROPVARIANT;
#define ERROR_FILE_CORRUPT 1392
#define ERROR_INVALID_DATA 13
#define ERROR_HANDLE_EOF 38
#define ERROR_NOT_SUPPORTED 50
#define HRESULT_FROM_WIN32(x) ((HRESULT)(((x) & 0xFFFF) | 0x80070000))
struct SYSTEMTIME { WORD wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds; };
inline BOOL TzSpecificLocalTimeToSystemTime(const void*, const SYSTEMTIME* l, SYSTEMTIME* s){ *s=*l; return TRUE; }
inline BOOL SystemTimeToFileTime(const SYSTEMTIME* s, FILETIME* f){ f->dwLowDateTime=0; f->dwHighDateTime=0; (void)s; return TRUE; }
