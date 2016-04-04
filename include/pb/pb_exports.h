#ifndef PB_EXPORTS_H
#define PB_EXPORTS_H

/** Stolen shamelessly from SDL2. Defines exports and calling convention for functions in the library. */
#ifndef DECLSPEC
# if defined(__WIN32__) || defined(__WINRT__)
#  ifdef __BORLANDC__
#   ifdef BUILD_LIBPB
#    define DECLSPEC
#   else
#    define DECLSPEC    __declspec(dllimport)
#   endif
#  else
#   define DECLSPEC __declspec(dllexport)
#  endif
# else
#  if defined(__GNUC__) && __GNUC__ >= 4
#   define DECLSPEC __attribute__ ((visibility("default")))
#  else
#   define DECLSPEC
#  endif
# endif
#endif

/* Use the C calling convention */
#ifndef PBCALL
#if (defined(__WIN32__) || defined(__WINRT__)) && !defined(__GNUC__)
#define PBCALL __cdecl
#else
#define PBCALL
#endif
#endif /* PBCALL */

#endif /* EXPORTS_H */