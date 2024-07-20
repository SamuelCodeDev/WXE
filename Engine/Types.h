#ifndef TYPES_H
#define TYPES_H

#ifndef _WIN32
	#include <cstdint>
#endif

#include <string_view>
#include <string>

namespace WXE
{
#ifdef _WIN32
	using int8 = __int8;
	using int16 = __int16;
	using int32 = __int32;
	using int64 = __int64;
	using uint8 = unsigned __int8;
	using uint16 = unsigned __int16;
	using uint32 = unsigned __int32;
	using uint64 = unsigned __int64;
#elif __linux__
	using int8 = int8_t;
	using int16 = int16_t;
	using int32 = int32_t;
	using int64 = int64_t;
	using uint8 = uint8_t;
	using uint16 = uint16_t;
	using uint32 = uint32_t;
	using uint64 = uint64_t;
#endif

	using string = std::string;
	using string_view = std::string_view;

	union Rect
	{
		struct
		{
			int32 left;
			int32 top;
			int32 right;
			int32 bottom;
		};

		int32 coord[4];
	};

	struct ViewPort
	{
		float TopLeftX;
		float TopLeftY;
		float Width;
		float Height;
		float MinDepth;
		float MaxDepth;
	};
}

#endif