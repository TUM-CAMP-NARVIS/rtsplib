#pragma once

#ifdef _WIN32
#   ifdef RTSPLIB_DLL
#       define RTSPLIB_EXPORT __declspec( dllexport )
#   else
#       define RTSPLIB_EXPORT __declspec( dllimport )
#   endif
#else // _WIN32
#   define RTSPLIB_EXPORT
#endif