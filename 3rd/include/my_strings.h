#ifndef MY_STRINGS_H
#define MY_STRINGS_H

#ifdef _WIN32
#include <windows.h>
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#else
#include <strings.h>
#endif

#endif/*MY_STRINGS_H*/
