#ifndef UTILS_H
#define UTILS_H

#ifndef _MSC_VER
#include <cstring>

template<typename T>
inline void* ZeroMemory(T * Destination, const size_t Length)
{ return memset(Destination, 0, Length); }

template<typename T>
inline void* CopyMemory(T * Destination, T * Source, const size_t Length)
{ return memcpy(Destination, Source, Length); }
#endif

template<typename T, size_t N>
inline constexpr size_t countof(T(&array)[N])
{ return N; }

template<typename T>
inline void SafeRelease(T * pointer)
{ if (pointer) pointer->Release(); }

#endif