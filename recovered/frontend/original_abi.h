#ifndef HD2_FRONTEND_ORIGINAL_ABI_H
#define HD2_FRONTEND_ORIGINAL_ABI_H
// The original target is Win32 i386. Native audits use their host ABI.
#if defined(_M_IX86)
#define HD2_CLIENT_THISCALL __thiscall
#define HD2_CLIENT_STDCALL __stdcall
#define HD2_CLIENT_FASTCALL __fastcall
#define HD2_CLIENT_BODY __declspec(noinline)
#elif defined(__i386__)
#define HD2_CLIENT_THISCALL __attribute__((thiscall))
#define HD2_CLIENT_STDCALL __attribute__((stdcall))
#define HD2_CLIENT_FASTCALL __attribute__((fastcall))
#define HD2_CLIENT_BODY __attribute__((noinline))
#else
#define HD2_CLIENT_THISCALL
#define HD2_CLIENT_STDCALL
#define HD2_CLIENT_FASTCALL
#define HD2_CLIENT_BODY
#endif
#endif
