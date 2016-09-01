#ifndef PB_UTIL_EXPORTS_H
#define PB_UTIL_EXPORTS_H

/* Stolen from SDL2 (and modified to actually detect _WIN32). Defines exports and calling convention for functions in the library. */
#ifndef PB_UTIL_DECLSPEC
# if defined(_WIN32) || defined(__WINRT__)
#   ifdef __BORLANDC__
#     ifdef BUILD_LIBPB_UTIL
#       define PB_UTIL_DECLSPEC
#     else
#       define PB_UTIL_DECLSPEC    __declspec(dllimport)
#     endif
#  else
#    ifdef libpb_util_EXPORTS
#      define PB_UTIL_DECLSPEC __declspec(dllexport)
#    else
#      define PB_UTIL_DECLSPEC __declspec(dllimport)
#    endif
#  endif
# else
#  if defined(__GNUC__) && __GNUC__ >= 4
#   define PB_UTIL_DECLSPEC __attribute__ ((visibility("default")))
#  else
#   define PB_UTIL_DECLSPEC
#  endif
# endif
#endif

/* Use the C calling convention */
#ifndef PB_UTIL_CALL
#if (defined(_WIN32) || defined(_WINRT) && !defined(__GNUC__))
#define PB_UTIL_CALL __cdecl
#else
#define PB_UTIL_CALL
#endif
#endif /* PBCALL */

#endif /* PB_UTIL_EXPORTS_H */